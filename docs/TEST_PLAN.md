# TEST_PLAN.md — Plano de Testes (Bancada + ROS)  
**Firmware AMR (RP2040) • FreeRTOS • micro-ROS • Dual-core • Multi-backend**

---

## 1) Objetivo do plano de testes

Validar o firmware em fases, com foco em:

1. **Determinismo** (100 Hz, jitter, deadline)  
2. **Observabilidade** (trace pins + logs)  
3. **Integração ROS** (micro-ROS via USB, tópicos e QoS)  
4. **Robustez** (watchdogs, falhas induzidas, estados)  
5. **Portabilidade** (troca de backend sem regressões)  

> Controle fechado (PI/Fuzzy) fica propositalmente para as fases finais.

---

## 2) Ferramentas e preparação de bancada

### 2.1 Ferramentas
- Osciloscópio e/ou analisador lógico (8 canais recomendável)
- Fonte com corrente limitada (para testes com motor)
- PC com ROS 2 Jazzy + micro-ROS agent
- Cabos USB para Pico (dados)
- (Opcional) adaptador UART-USB para CLI fallback

### 2.2 Preparação de sinais (GPIO trace)
Conecte os pinos de trace ao analisador:
- PIN0: loop begin
- PIN1: loop end
- PIN2: cmd_vel callback
- PIN3: publish odom
- PIN4: state change
- PIN5: fault/estop

**Antes de testar:** confirme que esses pinos não conflitam com SPI/UART/PWM.

### 2.3 Comandos úteis no host
- micro-ROS agent (USB CDC):
  - `micro_ros_agent serial --dev /dev/ttyACM0 -b 115200`
- ROS tools:
  - `ros2 topic list`
  - `ros2 topic echo /amr/status`
  - `ros2 topic hz /odom`
  - `ros2 topic pub /cmd_vel geometry_msgs/Twist "{linear: {x: 0.1}, angular: {z: 0.0}}" -r 20`
  - `ros2 run teleop_twist_keyboard teleop_twist_keyboard`

---

## 3) Critérios globais de aprovação

- Loop de 100 Hz:
  - período médio: 10 ms ± 1%
  - jitter pico-a-pico: preferível < 1 ms (ajuste conforme evolução)
- micro-ROS:
  - `/cmd_vel` recebido sem travar o loop
  - `/odom` publicado em taxa estável (30–50 Hz)
- Segurança:
  - cmd timeout leva a IDLE
  - E_STOP desliga motor imediatamente
  - FAULT latching funciona e exige clear/reset (conforme regra definida)

---

## 4) Testes por fase

### FASE A — Build/Boot/Profiles (sem periféricos)

**A1. Boot e transição BOOT→IDLE**  
- Procedimento:
  1. Flash firmware debug
  2. Monitor serial/log
- Verificações:
  - Estado inicial BOOT
  - Em seguida IDLE
  - PIN4 (state change) pulsa nas transições
- Pass/Fail:
  - Passa se entra em IDLE sem travar em < 2s

**A2. Troca de profile/backends compila e roda**  
- Procedimento:
  1. Compilar 2 perfis diferentes (ex.: PIO+Sabertooth vs LS7366R+BTS7960)
  2. Flashar e observar BOOT/IDLE
- Verificações:
  - Sem alteração de camadas acima do HAL
- Pass/Fail:
  - Passa se ambos inicializam e reportam backend correto no `/status`/log

---

### FASE B — Observabilidade e RT (sem encoder/motor)

**B1. Loop 100 Hz estável (sem carga)**  
- Procedimento:
  1. Conectar analyzer em PIN0 e PIN1
  2. Rodar firmware com TaskControlTick ativa (mas sem encoder/motor)
- Verificações:
  - período entre pulsos de PIN0 ≈ 10 ms
  - largura do “tempo de execução” (PIN0→PIN1) consistente
- Métricas:
  - jitter (pico-a-pico)
  - exec_time médio e máximo
- Pass/Fail:
  - Passa se exec_time < 20% de Ts (inicial) e jitter aceitável

**B2. Stress de logging (sem quebrar RT)**  
- Procedimento:
  1. Ativar log a 50–100 Hz no ring buffer
  2. Flush a 20–50 Hz
  3. Observar analyzer
- Verificações:
  - jitter não piora significativamente
  - sem perda de task (sem “deadline miss”)
- Pass/Fail:
  - Passa se jitter permanece dentro do limite definido

---

### FASE C — micro-ROS mínimo (sem encoder/motor)

**C1. micro-ROS agent conecta via USB**  
- Procedimento:
  1. Rodar agent no host
  2. Conferir que o node micro-ROS aparece
- Verificações:
  - `ros2 topic list` contém `/amr/status` (e outros)
- Pass/Fail:
  - Passa se tópicos aparecem em < 10s após boot

**C2. Recebimento de /cmd_vel (teleop e pub)**  
- Procedimento:
  1. `ros2 topic pub /cmd_vel ... -r 20`
  2. Observar PIN2 (cmd callback) e logs
- Verificações:
  - PIN2 pulsa na recepção
  - `/amr/status` reflete “cmd received” e timestamps atualizam
- Pass/Fail:
  - Passa se callback ocorre sem degradar loop 100 Hz

**C3. Watchdog de /cmd_vel (timeout)**  
- Procedimento:
  1. Enviar `/cmd_vel` por 3–5s
  2. Parar publicação e medir tempo até IDLE
- Verificações:
  - transição RUN→IDLE após T_timeout (ex.: 200 ms)
  - flag `CMD_TIMEOUT` setada
- Pass/Fail:
  - Passa se transição ocorre dentro de ±20% de T_timeout

---

### FASE D — Odometria “stub” (sem encoder real)

**D1. Publicação /odom em taxa fixa**  
- Procedimento:
  1. Firmware publica `/odom` a 30–50 Hz
  2. `ros2 topic hz /odom`
- Verificações:
  - taxa estável
  - timestamps monotônicos
- Pass/Fail:
  - Passa se hz está dentro de ±10%

**D2. Frames e TF básicos (RViz)**  
- Procedimento:
  1. Abrir RViz
  2. Adicionar Odometry
- Verificações:
  - frame_id e child_frame_id coerentes (`odom`, `base_link`)
- Pass/Fail:
  - Passa se RViz não acusa frame inexistente/tempo inconsistente

---

### FASE E — Encoder backend PIO+DMA

**E1. Snapshot consistente (sem glitches)**  
- Procedimento:
  1. Girar roda manualmente
  2. Observar `count` e `ω` nos logs
- Verificações:
  - contagem monotônica
  - ω suave (após filtro)
  - flags de glitch/stale = 0
- Pass/Fail:
  - Passa se sem saltos espúrios em uso normal

**E2. Robustez a altas rotações (se possível)**  
- Procedimento:
  1. Girar com motor em open-loop baixo/médio (se driver já pronto)
  2. Aumentar até limite seguro
- Verificações:
  - sem perda de contagem
  - sem overflow inesperado
- Pass/Fail:
  - Passa se leitura permanece estável

**E3. Fault por encoder stale**  
- Procedimento:
  1. Desconectar sinal de encoder (ou simular parada)
  2. Manter RUN ativo
- Verificações:
  - transição para FAULT
  - flag apropriada no status
- Pass/Fail:
  - Passa se FAULT ocorre e motor fica desabilitado

---

### FASE F — Encoder backend LS7366R (SPI)

Repete E1–E3, mudando backend.

**F1. Snapshot SPI monotônico**  
**F2. Alta rotação**  
**F3. Falha por desconexão / erro SPI (simulado)**

Pass/Fail: mesmas regras.

---

### FASE G — Motor driver open-loop (Sabertooth packetized)

**G1. Enable/disable imediato (sem controle)**  
- Procedimento:
  1. Em IDLE, motor deve estar off
  2. Entrar em RUN e aplicar `u=0.1` por 2s
  3. Enviar STOP e verificar off
- Verificações:
  - comando aplicado e removido imediatamente
- Pass/Fail:
  - Passa se STOP/E_STOP cortam motor de forma confiável

**G2. E_STOP físico/lógico**  
- Procedimento:
  1. Acionar e-stop
  2. Medir tempo até motor cortar (osciloscópio no PWM/linha de comando ou observação)
- Verificações:
  - transição para E_STOP
  - comando motor zerado/disable
- Pass/Fail:
  - Passa se corte é imediato (definir meta, ex.: < 50 ms)

---

### FASE H — Motor driver open-loop (BTS7960)

Repete G1–G2.

---

### FASE I — Pipeline “Nav2-ready” (sem controle)

**I1. /cmd_vel → refs de roda (cinemática)**  
- Procedimento:
  1. Enviar `/cmd_vel` com v e w conhecidos
  2. Ver no status/log as refs `ωL_ref`, `ωR_ref`
- Verificações:
  - sinais coerentes (v>0, w=0 → refs iguais)
  - w>0 aumenta ωR e diminui ωL (diferencial)
- Pass/Fail:
  - Passa se as relações batem com a cinemática

**I2. /odom coerente com encoder**  
- Procedimento:
  1. Rodas giram em open-loop
  2. Ver se odom integra corretamente (x aumenta)
- Verificações:
  - `x` cresce para frente, `θ` muda em curva
- Pass/Fail:
  - Passa se comportamento qualitativo é correto e sem “saltos”

**I3. Perda de /cmd_vel com motor ativo**  
- Procedimento:
  1. Rodar teleop por 3–5s
  2. Matar o publisher
- Verificações:
  - timeout → IDLE
  - motor desabilita
- Pass/Fail:
  - Passa se segurança se mantém

---

## 5) Testes finais (antes do controle fechado)

### T1. Regressão cruzada de backends
- Compilar e testar:
  - PIO+Sabertooth
  - PIO+BTS7960
  - LS7366R+Sabertooth
  - LS7366R+BTS7960
- Critério:
  - mesmos testes A–I passam com mínimas adaptações

### T2. Stress de micro-ROS + logs
- Executar:
  - `/cmd_vel` a 30–50 Hz
  - `/odom` a 50 Hz
  - logs a 50–100 Hz (buffer)
- Critério:
  - loop 100 Hz mantém jitter dentro do limite
  - sem drops críticos

---

## 6) Quando começar controle (gate)
Só iniciar controle PI/PID quando:
- Todos os testes A–I passaram
- Jitter do loop 100 Hz sob micro-ROS está caracterizado
- Encoder e motor são confiáveis em open-loop
- Estados e watchdogs estão sólidos

---

## 7) Apêndice — Checklist rápido de bancada

- [ ] Trace pins conectados e verificados  
- [ ] micro_ros_agent roda e conecta  
- [ ] `/cmd_vel` chega e pulsa PIN2  
- [ ] `/odom` publica com taxa estável  
- [ ] timeout muda para IDLE  
- [ ] E_STOP corta motor  
- [ ] logs não quebram o loop  
- [ ] troca de backend não quebra as camadas superiores
