/*
 ******************************************************************************
 * @file    Menu.h
 * @author  Zoltan Hudak
 * @version
 * @date    04-July-2014
 * @brief   Menu system with push buttons
  ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 Zoltan Hudak <hudakz@inbox.com>
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * The Menu library was created to facilitate designs with a display and push buttons.
 * - number of menus and push buttons is practically not limited by the software
 * - function of individual push buttons varies depending on the selected menu
 *
 *    ---------------------------------
 *   |                                  |
 *   |   ----------------------------   |
 *   |  |                            |  |
 *   |  |                            |  |
 *   |  |                            |  |
 *   |  |         Display            |  |
 *   |  |                            |  |
 *   |  |                            |  |
 *   |  |                            |  |
 *   |   ----------------------------   |
 *   |       O      O     O     O       |
 *   |      Btn1   Btn2  Btn3  Btn4     |
 *   |                                  |
 *    ----------------------------------
 *
 * Each push button have to be connected to an mbed digital input pin and the ground.
 *
 *
 *     -------------------------
 *    |
 *    |      mbed board
 *    |
 *     -------------------------
 *        | Input pin
 *        |
 *        o
 *         / Button        ...
 *        o
 *        |
 *       ---
 *       GND
 *
 *
 * NOTE: When creating a MenuSystem the constructor automatically connects
 * an internal pull-up resistor to each PushButton input used by the MenuSytem.
 */


#ifndef MENU_H_
#define MENU_H_

#include "mbed.h"

typedef void (*MenuFnc_t) (void);

#include "platform/platform.h"

#if defined (DEVICE_INTERRUPTIN) || defined(DOXYGEN_ONLY)

#include "hal/gpio_api.h"
#include "hal/gpio_irq_api.h"
#include "platform/Callback.h"
#include "platform/mbed_critical.h"
#include "platform/mbed_toolchain.h"
#include "platform/NonCopyable.h"

class MenuSystem;

class PushButton : private NonCopyable<PushButton> {

public:
    bool  _pressed;

    /** Create an PushButton connected to the specified pin
     *
     *  @param pin InterruptIn pin to connect to
     */
    PushButton(PinName pin);
    virtual ~PushButton();

    /** Attach a function to call when a falling edge occurs on the input
     *
     *  @param func A pointer to a void function, or 0 to set as none
     */
    void fall(Callback<void()> func);

    /** Enable IRQ. This method depends on hw implementation, might enable one
     *  port interrupts. For further information, check gpio_irq_enable().
     */
    void enable_irq();

    /** Disable IRQ. This method depends on hw implementation, might disable one
     *  port interrupts. For further information, check gpio_irq_disable().
     */
    void disable_irq();

    static void _irq_handler(uint32_t id, gpio_irq_event event);

protected:
    gpio_t      gpio;
    gpio_irq_t  gpio_irq;

    Callback<void()> _fall;
};

#endif


class   Menu
{
    MenuFnc_t*      _onBtnPressed;
    MenuSystem*     _menuSystem;
    void            onBtnPressed(unsigned int i);
public:
    Menu(MenuSystem& menuSystem);
    void            attach(PushButton* button, MenuFnc_t onButtonPressed);

    friend          MenuSystem;
};

class MenuSystem
{
    PushButton**    _buttons;
    unsigned int    _btnCount;
    unsigned int    _btnIdx;
    volatile bool   _btnBouncing;
    int             _debounceTime;
    Timeout         _bouncingTimeout;
    Menu*           _activeMenu;
    unsigned int    btnCount(void);
    void            onBtnPressed();
    PushButton*     getButton(size_t idx);
    void            debounce(void);
public:
    MenuSystem(PushButton* buttons[], size_t buttonCount, int debounceTime = 300 /* milliseconds */);
    void            open(Menu& menu);
    void            handleButtons(void);
    Menu*           activeMenu(void);

    friend          Menu;
};
#endif /* MENU_H_ */
