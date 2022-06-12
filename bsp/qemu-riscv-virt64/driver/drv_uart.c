/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-20     bigmagic     first version
 */
#include <rthw.h>
#include <rtdevice.h>

#include "board.h"
#include "drv_uart.h"

#include <stdio.h>
#include "sbi.h"
#include "interrupt.h"


#define XUARTPS_FIFO_OFFSET 0x0000000c
#define XUARTPS_SR_OFFSET 0x0000000b
#define SR_TX_FULL  0x10
#define SR_RX_EMPTY 0x02

volatile uint32_t* xuart = (void *)0xe0000000; //(void *)0x40010000;

void xuart_putchar(uint8_t ch) {
    volatile uint32_t *tx = xuart + XUARTPS_FIFO_OFFSET;
      while((*((volatile unsigned char*)(xuart + XUARTPS_SR_OFFSET)) & SR_TX_FULL) != 0); 
        *tx = ch; 
}

int xuart_getchar() {
    if (*((volatile unsigned char*)(xuart + XUARTPS_SR_OFFSET)) & SR_RX_EMPTY) {
          return -1; 
            }
      int32_t ch = xuart[XUARTPS_FIFO_OFFSET];
        return ch; 
}

void uart_puts(char *s) {
      int i = 0;
          while (s[i]) {
                    if (s[i]=='\n') xuart_putchar('\r');
                          xuart_putchar(s[i++]);
                              }
}

void uart_putn(int n) {
      char buf[10], buf_r[10];
          int offset = 0;
              do {
                        buf[offset++] = n % 10 + '0';
                                n = n / 10;
                                    } while (n);

                  for (int i = 0; i < offset; ++i)
                          buf_r[offset-i-1] = buf[i]; buf_r[offset] = 0;

                  uart_puts(buf_r);
}


struct device_uart
{
    rt_ubase_t  hw_base;
    rt_uint32_t irqno;
};

static rt_err_t  rt_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg);
static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg);
static int       drv_uart_putc(struct rt_serial_device *serial, char c);
static int       drv_uart_getc(struct rt_serial_device *serial);

void virt_uart_init(void)
{
    uart_puts("In virt_uart_init, use pk puts\n");

    //http://byterunner.com/16550.html
    uart_write_reg(IER, 0x00);

    uint8_t lcr = uart_read_reg(LCR);
    uart_write_reg(LCR, lcr | (1 << 7));
    uart_write_reg(DLL, 0x03);
    uart_write_reg(DLM, 0x00);

    lcr = 0;
    uart_write_reg(LCR, lcr | (3 << 0));

    /*
     * enable receive interrupts.
     */
    uint8_t ier = uart_read_reg(IER);
    uart_write_reg(IER, ier | (1 << 0));
}

/*
 * UART interface
 */
static rt_err_t rt_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct device_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    serial->config = *cfg;

    return (RT_EOK);
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct device_uart *uart;

    uart = serial->parent.user_data;
    rt_uint32_t channel = 1;

    RT_ASSERT(uart != RT_NULL);
    RT_ASSERT(channel != 3);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        break;

    case RT_DEVICE_CTRL_SET_INT:
        break;
    }

    return (RT_EOK);
}

static int drv_uart_putc(struct rt_serial_device *serial, char c)
{
  xuart_putchar(c);
  return (int)c;
//    while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0);
//    return uart_write_reg(THR, c);
}

static int drv_uart_getc(struct rt_serial_device *serial)
{
    if (uart_read_reg(LSR) & LSR_RX_READY){
        return uart_read_reg(RHR);
    } else {
        return -1;
    }
    //return sbi_console_getchar();
}

static void rt_hw_uart_isr(int irqno, void *param)
{
    struct rt_serial_device *serial = (struct rt_serial_device*)param;
    rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
}

struct rt_serial_device  serial1;
struct device_uart       uart1;

const struct rt_uart_ops _uart_ops =
{
    rt_uart_configure,
    uart_control,
    drv_uart_putc,
    drv_uart_getc,
    RT_NULL
};

/*
 * UART Initiation
 */
int rt_hw_uart_init(void)
{
    struct rt_serial_device *serial;
    struct device_uart      *uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    uart_puts("In_virt_hw_uart_init,_use_pk_puts\n");
    uart_puts("space:  \n");

    {
        serial  = &serial1;
        uart    = &uart1;

        serial->ops              = &_uart_ops;
        serial->config           = config;
        serial->config.baud_rate = UART_DEFAULT_BAUDRATE;

        uart->hw_base   = UART_BASE;
        uart->irqno     = UART0_IRQ;

        virt_uart_init();

        uart_puts("After_virt_uart_init\n");
        uart_puts("space:  \n");

        rt_hw_serial_register(serial,
                              "uart",
                              RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_RDWR, // | RT_DEVICE_FLAG_INT_RX,
                              uart);
        // rt_hw_interrupt_install(uart->irqno, rt_hw_uart_isr, serial, "uart");

        uart_puts("After_hw_serial_register\n");
        uart_puts("space:  \n");
        // rt_hw_interrupt_umask(uart->irqno);
    }

    return 0;
}
