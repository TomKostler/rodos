#include <cstdint>
#include "call_constructors.h"
#include "cortex_m/cm4_fpu.h"
#include "hw/irq_handlers.h"
#include "peripheral_ctrl/clock_management/clock_initializers.h"
#include "clkgen_config.h"

int main(int, char**);

namespace RODOS {

extern "C" volatile uint32_t _estack[];
extern "C" volatile uint32_t _sidata[];
extern "C" volatile uint32_t _sdata[];
extern "C" volatile uint32_t _edata[];
extern "C" volatile uint32_t _sbss[];
extern "C" volatile uint32_t _ebss[];

void initDataSection(){
    volatile uint32_t* destPtr = _sdata;
    const volatile uint32_t* srcPtr = _sidata;
    while(reinterpret_cast<uintptr_t>(destPtr) < reinterpret_cast<uintptr_t>(_edata)){
        *destPtr = *srcPtr;
        destPtr++;
        srcPtr++;
    }
}

void initBssSection(){
    volatile uint32_t* destPtr = _sbss;
    while(reinterpret_cast<uintptr_t>(destPtr) < reinterpret_cast<uintptr_t>(_ebss)){
        *destPtr = 0;
        destPtr++;
    }
}

inline void memoryBarrier(){
    asm volatile("" : : : "memory");
}


extern "C" [[noreturn, gnu::used]] void Reset_Handler(){
    memoryBarrier();
    initDataSection();
    initBssSection();
    memoryBarrier();
    setupFPU();
    memoryBarrier();
    call_constructors();
    memoryBarrier();
    ClockInitializer::init(globalClockSetup);
    memoryBarrier();

    main(0, nullptr);

    while(1);
}


constinit const struct IRQTable {
    const volatile void* sp = _estack;


    void (*reset_handler)() = Reset_Handler; //Reset Handler
    void (*nmi_handler)() = NMI_Handler; //NMI Handler
    void (*hardfault_handler)() = HardFault_Handler; //Hard Fault Handler
    void (*memmanage_handler)() = MemManage_Handler; //MPU Fault Handler
    void (*busfault_handler)() = BusFault_Handler; //Bus Fault Handler
    void (*usagefault_handler)() = UsageFault_Handler; //Usage Fault Handler
    uintptr_t reserved0 = 0; //Reserved
    uintptr_t reserved1 = 0; //Reserved
    uintptr_t reserved2 = 0; //Reserved
    uintptr_t reserved3 = 0; //Reserved
    void (*svc_handler)() = SVC_Handler; //SVCall Handler
    void (*debugmon_handler)() = DebugMon_Handler; //Debug Monitor Handler
    uintptr_t reserved = 0; //Reserved
    void (*pendsv_handler)() = PendSV_Handler; //PendSV Handler
    void (*systick_handler)() = SysTick_Handler; //SysTick Handler


    uintptr_t alwaysZero0 = 0; // 0: Always 0
    uintptr_t alwaysZero1 = 0; // 1: Always 0
    uintptr_t alwaysZero2 = 0; // 2: Always 0
    uintptr_t alwaysZero3 = 0; // 3: Always 0
    uintptr_t alwaysZero4 = 0; // 4: Always 0
    uintptr_t alwaysZero5 = 0; // 5: Always 0
    uintptr_t alwaysZero6 = 0; // 6: Always 0
    uintptr_t alwaysZero7 = 0; // 7: Always 0
    uintptr_t alwaysZero8 = 0; // 8: Always 0
    uintptr_t alwaysZero9 = 0; // 9: Always 0
    uintptr_t alwaysZero10 = 0; //10: Always 0
    uintptr_t alwaysZero11 = 0; //11: Always 0
    uintptr_t alwaysZero12 = 0; //12: Always 0
    uintptr_t alwaysZero13 = 0; //13: Always 0
    uintptr_t alwaysZero14 = 0; //14: Always 0
    uintptr_t alwaysZero15 = 0; //15: Always 0
    void (*spi0_tx_irqhandler)() = SPI0_TX_IRQHandler; //16: SPI0 TX
    void (*spi0_rx_irqhandler)() = SPI0_RX_IRQHandler; //17: SPI0 RX
    void (*spi1_tx_irqhandler)() = SPI1_TX_IRQHandler; //18: SPI1 TX
    void (*spi1_rx_irqhandler)() = SPI1_RX_IRQHandler; //19: SPI1 RX
    void (*spi2_tx_irqhandler)() = SPI2_TX_IRQHandler; //20: SPI2 TX
    void (*spi2_rx_irqhandler)() = SPI2_RX_IRQHandler; //21: SPI2 RX
    void (*spi3_tx_irqhandler)() = SPI3_TX_IRQHandler; //22: SPI3 TX
    void (*spi3_rx_irqhandler)() = SPI3_RX_IRQHandler; //23: SPI3 RX
    void (*uart0_tx_irqhandler)() = UART0_TX_IRQHandler; //24: UART0 TX
    void (*uart0_rx_irqhandler)() = UART0_RX_IRQHandler; //25: UART0 RX
    void (*uart1_tx_irqhandler)() = UART1_TX_IRQHandler; //26: UART1 TX
    void (*uart1_rx_irqhandler)() = UART1_RX_IRQHandler; //27: UART1 RX
    void (*uart2_tx_irqhandler)() = UART2_TX_IRQHandler; //28: UART2 TX
    void (*uart2_rx_irqhandler)() = UART2_RX_IRQHandler; //29: UART2 RX
    void (*i2c0_ms_irqhandler)() = I2C0_MS_IRQHandler; //30: I2C0_MS
    void (*i2c0_sl_irqhandler)() = I2C0_SL_IRQHandler; //31: I2C0_SL
    void (*i2c1_ms_irqhandler)() = I2C1_MS_IRQHandler; //32: I2C1_MS
    void (*i2c1_sl_irqhandler)() = I2C1_SL_IRQHandler; //33: I2C1_SL
    void (*i2c2_ms_irqhandler)() = I2C2_MS_IRQHandler; //34: I2C2_MS
    void (*i2c2_sl_irqhandler)() = I2C2_SL_IRQHandler; //35: I2C2_SL
    void (*ether_tx_irqhandler)() = Ether_TX_IRQHandler; //36: Ethernet TX
    uintptr_t alwaysZero37 = 0; //37: Always 0
    void (*spw_irqhandler)() = SpW_IRQHandler; //38: Space Wire
    uintptr_t alwaysZero39 = 0; //39: Always 0
    void (*dac0_irqhandler)() = DAC0_IRQHandler; //40: DAC 0
    void (*dac1_irqhandler)() = DAC1_IRQHandler; //41: DAC 1
    void (*trng_irqhandler)() = TRNG_IRQHandler; //42: Random Number Generator
    void (*dma_error_irqhandler)() = DMA_Error_IRQHandler; //43: DMA error
    void (*adc_irqhandler)() = ADC_IRQHandler; //44: ADC
    void (*loclk_irqhandler)() = LoCLK_IRQHandler; //45: LoCLK
    void (*lvd_irqhandler)() = LVD_IRQHandler; //46: LVD
    void (*wdt_irqhandler)() = WDT_IRQHandler; //47: Watchdog
    void (*tim0_irqhandler)() = TIM0_IRQHandler; //48: Timer 0
    void (*tim1_irqhandler)() = TIM1_IRQHandler; //49: Timer 1
    void (*tim2_irqhandler)() = TIM2_IRQHandler; //50: Timer 2
    void (*tim3_irqhandler)() = TIM3_IRQHandler; //51: Timer 3
    void (*tim4_irqhandler)() = TIM4_IRQHandler; //52: Timer 4
    void (*tim5_irqhandler)() = TIM5_IRQHandler; //53: Timer 5
    void (*tim6_irqhandler)() = TIM6_IRQHandler; //54: Timer 6
    void (*tim7_irqhandler)() = TIM7_IRQHandler; //55: Timer 7
    void (*tim8_irqhandler)() = TIM8_IRQHandler; //56: Timer 8
    void (*tim9_irqhandler)() = TIM9_IRQHandler; //57: Timer 9
    void (*tim10_irqhandler)() = TIM10_IRQHandler; //58: Timer 10
    void (*tim11_irqhandler)() = TIM11_IRQHandler; //59: Timer 11
    void (*tim12_irqhandler)() = TIM12_IRQHandler; //60: Timer 12
    void (*tim13_irqhandler)() = TIM13_IRQHandler; //61: Timer 13
    void (*tim14_irqhandler)() = TIM14_IRQHandler; //62: Timer 14
    void (*tim15_irqhandler)() = TIM15_IRQHandler; //63: Timer 15
    void (*tim16_irqhandler)() = TIM16_IRQHandler; //64: Timer 16
    void (*tim17_irqhandler)() = TIM17_IRQHandler; //65: Timer 17
    void (*tim18_irqhandler)() = TIM18_IRQHandler; //66: Timer 18
    void (*tim19_irqhandler)() = TIM19_IRQHandler; //67: Timer 19
    void (*tim20_irqhandler)() = TIM20_IRQHandler; //68: Timer 20
    void (*tim21_irqhandler)() = TIM21_IRQHandler; //69: Timer 21
    void (*tim22_irqhandler)() = TIM22_IRQHandler; //70: Timer 22
    void (*tim23_irqhandler)() = TIM23_IRQHandler; //71: Timer 23
    void (*can0_irqhandler)() = CAN0_IRQHandler; //72: CAN 0
    uintptr_t alwaysZero73 = 0; //73: Always 0
    void (*can1_irqhandler)() = CAN1_IRQHandler; //74: CAN 1
    uintptr_t alwaysZero75 = 0; //75: Always 0
    void (*edac_mbe_irqhandler)() = EDAC_MBE_IRQHandler; //76: EDAC Multi Bit Error
    void (*edac_sbe_irqhandler)() = EDAC_SBE_IRQHandler; //77: EDAC Single Bit Error
    void (*pa0_irqhandler)() = PA0_IRQHandler; //78: PORTA 0
    void (*pa1_irqhandler)() = PA1_IRQHandler; //79: PORTA 1
    void (*pa2_irqhandler)() = PA2_IRQHandler; //80: PORTA 2
    void (*pa3_irqhandler)() = PA3_IRQHandler; //81: PORTA 3
    void (*pa4_irqhandler)() = PA4_IRQHandler; //82: PORTA 4
    void (*pa5_irqhandler)() = PA5_IRQHandler; //83: PORTA 5
    void (*pa6_irqhandler)() = PA6_IRQHandler; //84: PORTA 6
    void (*pa7_irqhandler)() = PA7_IRQHandler; //85: PORTA 7
    void (*pa8_irqhandler)() = PA8_IRQHandler; //86: PORTA 8
    void (*pa9_irqhandler)() = PA9_IRQHandler; //87: PORTA 9
    void (*pa10_irqhandler)() = PA10_IRQHandler; //88: PORTA 10
    void (*pa11_irqhandler)() = PA11_IRQHandler; //89: PORTA 11
    void (*pa12_irqhandler)() = PA12_IRQHandler; //90: PORTA 12
    void (*pa13_irqhandler)() = PA13_IRQHandler; //91: PORTA 13
    void (*pa14_irqhandler)() = PA14_IRQHandler; //92: PORTA 14
    void (*pa15_irqhandler)() = PA15_IRQHandler; //93: PORTA 15
    void (*pb0_irqhandler)() = PB0_IRQHandler; //94: PORTB 0
    void (*pb1_irqhandler)() = PB1_IRQHandler; //95: PORTB 1
    void (*pb2_irqhandler)() = PB2_IRQHandler; //96: PORTB 2
    void (*pb3_irqhandler)() = PB3_IRQHandler; //97: PORTB 3
    void (*pb4_irqhandler)() = PB4_IRQHandler; //98: PORTB 4
    void (*pb5_irqhandler)() = PB5_IRQHandler; //99: PORTB 5
    void (*pb6_irqhandler)() = PB6_IRQHandler; //100: PORTB 6
    void (*pb7_irqhandler)() = PB7_IRQHandler; //101: PORTB 7
    void (*pb8_irqhandler)() = PB8_IRQHandler; //102: PORTB 8
    void (*pb9_irqhandler)() = PB9_IRQHandler; //103: PORTB 9
    void (*pb10_irqhandler)() = PB10_IRQHandler; //104: PORTB 10
    void (*pb11_irqhandler)() = PB11_IRQHandler; //105: PORTB 11
    void (*pb12_irqhandler)() = PB12_IRQHandler; //106: PORTB 12
    void (*pb13_irqhandler)() = PB13_IRQHandler; //107: PORTB 13
    void (*pb14_irqhandler)() = PB14_IRQHandler; //108: PORTB 14
    void (*pb15_irqhandler)() = PB15_IRQHandler; //109: PORTB 15
    void (*pc0_irqhandler)() = PC0_IRQHandler; //110: PORTC 0
    void (*pc1_irqhandler)() = PC1_IRQHandler; //111: PORTC 1
    void (*pc2_irqhandler)() = PC2_IRQHandler; //112: PORTC 2
    void (*pc3_irqhandler)() = PC3_IRQHandler; //113: PORTC 3
    void (*pc4_irqhandler)() = PC4_IRQHandler; //114: PORTC 4
    void (*pc5_irqhandler)() = PC5_IRQHandler; //115: PORTC 5
    void (*pc6_irqhandler)() = PC6_IRQHandler; //116: PORTC 6
    void (*pc7_irqhandler)() = PC7_IRQHandler; //117: PORTC 7
    void (*pc8_irqhandler)() = PC8_IRQHandler; //118: PORTC 8
    void (*pc9_irqhandler)() = PC9_IRQHandler; //119: PORTC 9
    void (*pc10_irqhandler)() = PC10_IRQHandler; //120: PORTC 10
    void (*pc11_irqhandler)() = PC11_IRQHandler; //121: PORTC 11
    void (*pc12_irqhandler)() = PC12_IRQHandler; //122: PORTC 12
    void (*pc13_irqhandler)() = PC13_IRQHandler; //123: PORTC 13
    void (*pc14_irqhandler)() = PC14_IRQHandler; //124: PORTC 14
    void (*pc15_irqhandler)() = PC15_IRQHandler; //125: PORTC 15
    void (*pd0_irqhandler)() = PD0_IRQHandler; //126: PORTD 0
    void (*pd1_irqhandler)() = PD1_IRQHandler; //127: PORTD 1
    void (*pd2_irqhandler)() = PD2_IRQHandler; //128: PORTD 2
    void (*pd3_irqhandler)() = PD3_IRQHandler; //129: PORTD 3
    void (*pd4_irqhandler)() = PD4_IRQHandler; //130: PORTD 4
    void (*pd5_irqhandler)() = PD5_IRQHandler; //131: PORTD 5
    void (*pd6_irqhandler)() = PD6_IRQHandler; //132: PORTD 6
    void (*pd7_irqhandler)() = PD7_IRQHandler; //133: PORTD 7
    void (*pd8_irqhandler)() = PD8_IRQHandler; //134: PORTD 8
    void (*pd9_irqhandler)() = PD9_IRQHandler; //135: PORTD 9
    void (*pd10_irqhandler)() = PD10_IRQHandler; //136: PORTD 10
    void (*pd11_irqhandler)() = PD11_IRQHandler; //137: PORTD 11
    void (*pd12_irqhandler)() = PD12_IRQHandler; //138: PORTD 12
    void (*pd13_irqhandler)() = PD13_IRQHandler; //139: PORTD 13
    void (*pd14_irqhandler)() = PD14_IRQHandler; //140: PORTD 14
    void (*pd15_irqhandler)() = PD15_IRQHandler; //141: PORTD 15
    void (*pe0_irqhandler)() = PE0_IRQHandler; //142: PORTE 0
    void (*pe1_irqhandler)() = PE1_IRQHandler; //143: PORTE 1
    void (*pe2_irqhandler)() = PE2_IRQHandler; //144: PORTE 2
    void (*pe3_irqhandler)() = PE3_IRQHandler; //145: PORTE 3
    void (*pe4_irqhandler)() = PE4_IRQHandler; //146: PORTE 4
    void (*pe5_irqhandler)() = PE5_IRQHandler; //147: PORTE 5
    void (*pe6_irqhandler)() = PE6_IRQHandler; //148: PORTE 6
    void (*pe7_irqhandler)() = PE7_IRQHandler; //149: PORTE 7
    void (*pe8_irqhandler)() = PE8_IRQHandler; //150: PORTE 8
    void (*pe9_irqhandler)() = PE9_IRQHandler; //151: PORTE 9
    void (*pe10_irqhandler)() = PE10_IRQHandler; //152: PORTE 10
    void (*pe11_irqhandler)() = PE11_IRQHandler; //153: PORTE 11
    void (*pe12_irqhandler)() = PE12_IRQHandler; //154: PORTE 12
    void (*pe13_irqhandler)() = PE13_IRQHandler; //155: PORTE 13
    void (*pe14_irqhandler)() = PE14_IRQHandler; //156: PORTE 14
    void (*pe15_irqhandler)() = PE15_IRQHandler; //157: PORTE 15
    void (*pf0_irqhandler)() = PF0_IRQHandler; //158: PORTF 0
    void (*pf1_irqhandler)() = PF1_IRQHandler; //159: PORTF 1
    void (*pf2_irqhandler)() = PF2_IRQHandler; //160: PORTF 2
    void (*pf3_irqhandler)() = PF3_IRQHandler; //161: PORTF 3
    void (*pf4_irqhandler)() = PF4_IRQHandler; //162: PORTF 4
    void (*pf5_irqhandler)() = PF5_IRQHandler; //163: PORTF 5
    void (*pf6_irqhandler)() = PF6_IRQHandler; //164: PORTF 6
    void (*pf7_irqhandler)() = PF7_IRQHandler; //165: PORTF 7
    void (*pf8_irqhandler)() = PF8_IRQHandler; //166: PORTF 8
    void (*pf9_irqhandler)() = PF9_IRQHandler; //167: PORTF 9
    void (*pf10_irqhandler)() = PF10_IRQHandler; //168: PORTF 10
    void (*pf11_irqhandler)() = PF11_IRQHandler; //169: PORTF 11
    void (*pf12_irqhandler)() = PF12_IRQHandler; //170: PORTF 12
    void (*pf13_irqhandler)() = PF13_IRQHandler; //171: PORTF 13
    void (*pf14_irqhandler)() = PF14_IRQHandler; //172: PORTF 14
    void (*pf15_irqhandler)() = PF15_IRQHandler; //173: PORTF 15
    void (*dma_active_0_irqhandler)() = DMA_Active_0_IRQHandler; //174: DMA Active 0
    void (*dma_active_1_irqhandler)() = DMA_Active_1_IRQHandler; //175: DMA Active 1
    void (*dma_active_2_irqhandler)() = DMA_Active_2_IRQHandler; //176: DMA Active 2
    void (*dma_active_3_irqhandler)() = DMA_Active_3_IRQHandler; //177: DMA Active 3
    void (*dma_done_0_irqhandler)() = DMA_Done_0_IRQHandler; //178: DMA Done 0
    void (*dma_done_1_irqhandler)() = DMA_Done_1_IRQHandler; //179: DMA Done 1
    void (*dma_done_2_irqhandler)() = DMA_Done_2_IRQHandler; //180: DMA Done 2
    void (*dma_done_3_irqhandler)() = DMA_Done_3_IRQHandler; //181: DMA Done 3
    void (*i2c0_ms_rx_irqhandler)() = I2C0_MS_RX_IRQHandler; //182: I2C0 Master RX
    void (*i2c0_ms_tx_irqhandler)() = I2C0_MS_TX_IRQHandler; //183: I2C0 Master TX
    void (*i2c0_sl_rx_irqhandler)() = I2C0_SL_RX_IRQHandler; //184: I2C0 Slave RX
    void (*i2c0_sl_tx_irqhandler)() = I2C0_SL_TX_IRQHandler; //185: I2C0 Slave TX
    void (*i2c1_ms_rx_irqhandler)() = I2C1_MS_RX_IRQHandler; //186: I2C1 Master RX
    void (*i2c1_ms_tx_irqhandler)() = I2C1_MS_TX_IRQHandler; //187: I2C1 Master TX
    void (*i2c1_sl_rx_irqhandler)() = I2C1_SL_RX_IRQHandler; //188: I2C1 Slave RX
    void (*i2c1_sl_tx_irqhandler)() = I2C1_SL_TX_IRQHandler; //189: I2C1 Slave TX
    void (*i2c2_ms_rx_irqhandler)() = I2C2_MS_RX_IRQHandler; //190: I2C2 Master RX
    void (*i2c2_ms_tx_irqhandler)() = I2C2_MS_TX_IRQHandler; //191: I2C2 Master TX
    void (*i2c2_sl_rx_irqhandler)() = I2C2_SL_RX_IRQHandler; //192: I2C2 Slave RX
    void (*i2c2_sl_tx_irqhandler)() = I2C2_SL_TX_IRQHandler; //193: I2C2 Slave TX
    void (*fpu_irqhandler)() = FPU_IRQHandler; //194: FPU
    void (*txev_irqhandler)() = TXEV_IRQHandler; //195: TXEV
} irqTable __attribute__((used, section(".isr_vector")));


}
