# ARCHITECTURE.md — Firmware AMR (RP2040)  
**Raspberry Pi Pico • FreeRTOS • micro-ROS (USB CDC) • Dual-core • Multi-backend HAL**

---

## 1) Objetivos de arquitetura

1. **Determinismo no tempo real** (controle/estimação/safety em 100 Hz)  
2. **Observabilidade total** (logs em software + marcadores em hardware)  
3. **Integração ROS/Nav2 primeiro** (micro-ROS como interface principal)  
4. **Reutilização multi-robô** (troca de encoder/driver por *profiles* e backends)  
5. **Isolamento de jitter** (separação por core e budgets fixos)  
6. **Segurança operacional** (máquina de estados + watchdogs)

---

## 2) Visão geral em blocos

```
            (Core 0)                                   (Core 1)
┌───────────────────────────────┐            ┌───────────────────────────────┐
│ Comunicação & Observabilidade  │            │ Tempo Real (Hard RT)          │
│                               │            │                               │
│  micro-ROS executor           │            │  100 Hz loop (TaskControlTick)│
│  /cmd_vel subscriber          │  setpoints │  encoder snapshot              │
│  /odom + /status publishers   │──────────► │  estimação de velocidade       │
│  logger (ring buffer flush)   │            │  safety + watchdogs            │
│  CLI debug (UART opcional)    │ ◄───────── │  telemetria (snapshot)         │
└───────────────────────────────┘  telemetry  └───────────────────────────────┘

                 HAL (backends selecionáveis por build/profile)
     Encoders: PIO+DMA  OR  LS7366R(SPI)       Motores: Sabertooth OR BTS7960
```

---

## 3) Separação por camadas (Clean-ish layers)

### 3.1 Camadas e responsabilidades

**A) HAL (Hardware Abstraction Layer)**  
- Encoders: `PIO+DMA` (quadratura) ou `LS7366R SPI`  
- Motor: `Sabertooth packetized UART` ou `BTS7960 PWM+DIR`  
- Tempo: timers, `time_us`, alarm/ticks  
- GPIO: enable, estop, LEDs, trace pins  
- UART/USB: debug (mínimo), transporte (USB CDC para micro-ROS)  

**B) Estimação**  
- Conversão `counts -> rad/s` por janela fixa (100 Hz)  
- Filtro (IIR 1ª ordem ou média)  
- Odometria diferencial (x,y,θ)  
- Consistência temporal (timestamp do snapshot)

**C) Controle (última fase do roadmap)**  
- PI/PID com anti-windup  
- Rampa (slew-rate) em referências  
- Saturações e limites  
- Interface comum para plugar Fuzzy depois

**D) Comunicação**  
- micro-ROS node (sub /cmd_vel, pub /odom, /status)  
- QoS e budgets fixos  
- CLI UART simples para bringup / fallback (opcional)

**E) Supervisão**  
- Máquina de estados (BOOT, IDLE, RUN, FAULT, E_STOP)  
- Watchdogs: cmd_vel, encoder, loop deadline  
- Ações seguras: disable motor, transições, latch faults

---

## 4) Interfaces estáveis (contratos)

### 4.1 Encoder HAL (independente do backend)

```c
typedef struct {
  int32_t count_l;
  int32_t count_r;
  uint32_t t_us;      // timestamp do snapshot
  uint32_t flags;     // overflow/glitch/stale
} encoder_snapshot_t;

bool hal_encoder_init(void);
bool hal_encoder_get_snapshot(encoder_snapshot_t* out);
encoder_health_t hal_encoder_health(void);
```

**Garantias da interface**
- `get_snapshot()` retorna contagens **monotônicas** (salvo overflow detectado)
- snapshot é **consistente** (não “meio atualizado”) — ping/pong no DMA ou leitura atômica no SPI
- timestamp válido para derivar velocidade por janela

### 4.2 Motor HAL (independente do backend)

```c
bool hal_motor_init(void);
void hal_motor_enable(bool en);
void hal_motor_set_u(float u_l, float u_r); // [-1,1]
motor_health_t hal_motor_health(void);
```

**Garantias**
- `enable(false)` desliga motor **imediatamente**
- saturação e deadband são responsabilidade do HAL (por backend)

---

## 5) Profiles e seleção de backends (multi-robô)

### 5.1 Profile
Um *profile* define: backends + pinagem + defaults.

Exemplo conceitual:
- `profiles/amr_diff_sabertooth_pio.h`
- `profiles/amr_diff_bts7960_ls7366r.h`

### 5.2 Seleção por build
- `ROBOT_PROFILE=<nome>`
- `ENCODER_BACKEND=PIO_DMA|LS7366R_SPI`
- `MOTOR_BACKEND=SABERTOOTH|BTS7960`

**Regra:** camadas acima do HAL não mudam.

---

## 6) Máquina de estados (segurança e previsibilidade)

### 6.1 Estados

- **BOOT**: init HAL, configs, tasks, self-check → IDLE  
- **IDLE**: motor desabilitado; aceita comandos; odom pode manter (opcional)  
- **RUN**: recebe setpoints; estimação+controle+watchdogs ativos  
- **FAULT**: motor desabilitado; latch de falha; sai com `clear_fault`  
- **E_STOP**: motor desabilitado “hard”; sai por condição de liberação (ou reset)

### 6.2 Eventos típicos
- `EV_START`, `EV_STOP`
- `EV_CMD_TIMEOUT`
- `EV_ENCODER_STALE`, `EV_ENCODER_GLITCH`, `EV_STALL`
- `EV_LOOP_DEADLINE_MISS`
- `EV_ESTOP_ASSERT`, `EV_ESTOP_RELEASE`
- `EV_CLEAR_FAULT`

---

## 7) Watchdogs (o que é observado)

### 7.1 cmd_vel watchdog (ROS)
- Se `/cmd_vel` não chega por **T_timeout** (ex.: 200 ms):  
  - RUN → IDLE  
  - zera referências (com rampa ou stop imediato conforme config)  
  - seta flag `CMD_TIMEOUT`

### 7.2 Encoder watchdog
- snapshot stale (tempo sem atualização) → FAULT  
- salto absurdo de contagem → FAULT  
- referência alta e Δcount ~ 0 por N ciclos → FAULT (stall)

### 7.3 Loop watchdog (deadline)
- mede duração do ciclo do 100 Hz  
- se `exec_time > Ts + margem` por N ocorrências → FAULT

---

## 8) Dual-core + FreeRTOS: tasks e prioridades

### 8.1 Core 1 (hard real-time)
- **TaskControlTick @100 Hz** (prio máxima)  
  - snapshot encoder  
  - estimação (ωL/ωR)  
  - (controle: mais tarde)  
  - aplica `hal_motor_set_u()` (ou open-loop)  
  - atualiza telemetria compartilhada  
  - trace pins begin/end

- **TaskSafety @100 Hz** (prio alta)  
  - e-stop, watchdogs, transições de estado

### 8.2 Core 0 (comunicação e observabilidade)
- **TaskMicroROS** (prio média-alta)  
  - `rclc_executor_spin_some()` com budget fixo  
  - callback `/cmd_vel` atualiza setpoint compartilhado  
  - trace pin “cmd received”

- **TaskPublish @30–50 Hz**  
  - publica `/odom` e `/status`

- **TaskLogger @20–50 Hz**  
  - drena ring buffer e envia via USB CDC (fora do loop 100 Hz)

- **TaskCLI (opcional)**  
  - comandos simples para bringup/fallback

---

## 9) Comunicação entre cores (setpoints/telemetria)

### 9.1 Core0 → Core1 (setpoints)
Conteúdo mínimo:
- `v_cmd` [m/s], `w_cmd` [rad/s] **ou** `ωL_ref`, `ωR_ref` [rad/s]
- `enable/run` e timestamp do último cmd

Implementação recomendada:
- *single-writer/single-reader* com estrutura “double buffer” e sequência
- ou `FreeRTOS queue` (curta, sem bloqueio no consumidor)

### 9.2 Core1 → Core0 (telemetria)
Conteúdo:
- `ωL_meas`, `ωR_meas`, `uL`, `uR`
- `x,y,θ` (odom)
- `state`, `flags`, counters watchdog
- timestamps

---

## 10) Tópicos ROS (contrato com Nav2)

### 10.1 Subscriptions
- `/cmd_vel` : `geometry_msgs/Twist`  
  - usa `linear.x` e `angular.z`

### 10.2 Publications
- `/odom` : `nav_msgs/Odometry`  
- `/amr/status` : msg custom (ou `diagnostic_msgs`)  
- (opcional) `/joint_states` : `sensor_msgs/JointState`

### 10.3 QoS recomendado
- `/cmd_vel`: best-effort, depth 1  
- `/odom`: best-effort, depth 5–10  
- `/status`: reliable (opcional), depth 10

---

## 11) Instrumentação (debug + medição)

### 11.1 Hardware trace pins (exemplo)
- PIN0: loop 100Hz begin  
- PIN1: loop 100Hz end  
- PIN2: cmd_vel callback  
- PIN3: publish odom  
- PIN4: state change  
- PIN5: fault/estop

### 11.2 Software logging
- ring buffer binário interno  
- export CSV no host
- “snapshot de falha”: últimos N segundos antes de FAULT/E_STOP

---

## 12) Convenções de unidades (evita bugs)
- `v_cmd` em **m/s**
- `w_cmd` em **rad/s**
- `ω` (rodas) em **rad/s**
- `u` normalizado em **[-1,1]**
- `t_us` em **microsegundos**
- encoder: `counts_per_motor_rev` e `gear_ratio` explícitos

---

## 13) Estrutura sugerida do projeto

```
firmware/
  src/
    app/          (state machine, supervisor, config)
    hal/          (encoder backends, motor backends, trace pins)
    estimation/   (velocity, odometry, filters)
    control/      (PI/PID/Fuzzy - fase final)
    comm/         (micro-ROS node, publishers, CLI)
    utils/        (ring buffer, crc, math helpers)
  profiles/
  docs/
```

---

## 14) Critérios de sucesso (antes do controle)
- Loop 100 Hz com jitter baixo confirmado no analyzer  
- micro-ROS estável via USB, recebendo `/cmd_vel` sem degradar o loop  
- `/odom` coerente, frames e timestamps corretos  
- Watchdogs e estados confiáveis  
- Troca de backend (PIO vs LS7366R, Sabertooth vs BTS7960) sem alterar camadas acima do HAL
