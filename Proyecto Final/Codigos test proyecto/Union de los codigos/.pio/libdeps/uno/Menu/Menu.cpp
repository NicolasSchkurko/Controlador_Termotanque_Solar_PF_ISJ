/*
 ******************************************************************************
 * @file    Menu.cpp
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
 * Each push button must be connected to an mbed digital input pin and the ground.
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
 * an internal pull-up resistor to each InterruptIn input used by the MenuSytem.
 */
#include "Menu.h"

/*
 ***********************************************************************
 **************************** PushButton *******************************
 ***********************************************************************
 */
/**
 * @brief
 * @note
 * @param
 * @retval
 */
PushButton::PushButton(PinName pin)
    : _pressed(false), gpio(), gpio_irq(), _fall(NULL) {
    // No lock needed in the constructor
    gpio_irq_init(&gpio_irq, pin, (&PushButton::_irq_handler), (uint32_t)this);
    gpio_init_in(&gpio, pin);
    gpio_mode(&gpio, PullUp);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
PushButton::~PushButton() {
    // No lock needed in the destructor
    gpio_irq_free(&gpio_irq);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void PushButton::fall(Callback<void()> func) {
    core_util_critical_section_enter();
    if (func) {
        _fall = func;
        gpio_irq_set(&gpio_irq, IRQ_FALL, 1);
    } else {
        _fall = NULL;
        gpio_irq_set(&gpio_irq, IRQ_FALL, 0);
    }
    core_util_critical_section_exit();
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void PushButton::_irq_handler(uint32_t id, gpio_irq_event event) {
    PushButton *handler = (PushButton*)id;
    switch (event) {
        case IRQ_FALL:
            if (handler->_fall) {
                handler->_pressed = true;
                handler->_fall();
            }
            break;
        case IRQ_RISE:
        case IRQ_NONE:
            break;
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void PushButton::enable_irq() {
    core_util_critical_section_enter();
    gpio_irq_enable(&gpio_irq);
    core_util_critical_section_exit();
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void PushButton::disable_irq() {
    core_util_critical_section_enter();
    gpio_irq_disable(&gpio_irq);
    core_util_critical_section_exit();
}

/*
 ***********************************************************************
 ****************************** Menu ***********************************
 ***********************************************************************
 */

/**
 * @brief
 * @note
 * @param
 * @retval
 */
Menu::Menu(MenuSystem& menuSystem) :
    _menuSystem(&menuSystem) {
    /*-----------------------------------*/
    int btnCount = _menuSystem->btnCount();
    /*-----------------------------------*/

    _onBtnPressed = (MenuFnc_t*)malloc(sizeof(MenuFnc_t) * btnCount);
    for (int i = 0; i < btnCount; i++)
        _onBtnPressed[i] = NULL;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void Menu::attach(PushButton* button, MenuFnc_t onButtonPressed) {
    /*-----------------------------------*/
    int btnCount = _menuSystem->btnCount();
    /*-----------------------------------*/

    for (int i = 0; i < btnCount; i++) {
        /*------------------------------------------*/
        PushButton*  p_btn = _menuSystem->getButton(i);
        /*------------------------------------------*/

        if (p_btn == button) {
            _onBtnPressed[i] = onButtonPressed;
            break;
        }
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void Menu::onBtnPressed(unsigned int i) {
    if (_onBtnPressed[i] != NULL)
        _onBtnPressed[i]();
}

/*
 ***********************************************************************
 **************************** MenuSystem *******************************
 ***********************************************************************
 */

/**
 * @brief
 * @note
 * @param
 * @retval
 */
MenuSystem::MenuSystem(PushButton* buttons[], size_t buttonCount, int debounceTime /*=300ms*/ ) {
    _buttons = buttons;
    _btnCount = buttonCount;
    _btnBouncing = false;
    _debounceTime = debounceTime;
    _activeMenu = NULL;
    for (size_t i = 0; i < _btnCount; i++)
        _buttons[i]->fall(callback(this, &MenuSystem::onBtnPressed));
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
unsigned int MenuSystem::btnCount(void) {
    return(_btnCount);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void MenuSystem::open(Menu& menu) {
    _activeMenu = &menu;
}

void MenuSystem::onBtnPressed() {
    _btnBouncing = true;
    _bouncingTimeout.attach_us(callback(this, &MenuSystem::debounce), _debounceTime * 1000.0f);
}

/**
 * @brief   Handles "button pressed" events.
 * @note    Buttons are debounced in ISR.
 * @param   None
 * @retval  None
 */
void MenuSystem::handleButtons(void) {
    for (unsigned int i = 0; i < _btnCount; i++) {
        if (_buttons[i]->_pressed && !_btnBouncing){
            _buttons[i]->_pressed = false;
            _activeMenu->onBtnPressed(i);
            return;
        }
    }
}

/**
 * @brief   Re-enables buttons when bouncing is over.
 * @note    None
 * @param   None
 * @retval  None
 */
void MenuSystem::debounce(void) {
    _btnBouncing = false;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
Menu* MenuSystem::activeMenu(void) {
    return _activeMenu;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
PushButton* MenuSystem::getButton(size_t i) {
    return _buttons[i];
}

