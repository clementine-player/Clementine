/**********************************************************************************
 * Wiimotedev daemon headers                                                      *
 * Copyright (C) 2010  Bartłomiej Burdukiewicz                                    *
 * Contact: dev.strikeu@gmail.com                                                 *
 *                                                                                *
 * This program is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                     *
 * License as published by the Free Software Foundation; either                   *
 * version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                *
 * This program is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 * Lesser General Public License for more details.                                *
 *                                                                                *
 * You should have received a copy of the GNU Lesser General Public               *
 * License along with this program; if not, see <http://www.gnu.org/licences/>.   *
 **********************************************************************************/


#ifndef WIIMOTEDEVPROTO_H
#define WIIMOTEDEVPROTO_H

/* Typedefs ------------------------------------------------------ */
typedef unsigned short uint16;

/* Proto ids ----------------------------------------------------- */
const uint16 iddbusWiimoteGeneralButtons = 0x01;
const uint16 iddbusWiimoteConnected= 0x02;
const uint16 iddbusWiimoteDisconnected = 0x03;
const uint16 iddbusWiimoteBatteryLife = 0x04;
const uint16 iddbusWiimoteButtons = 0x05;
const uint16 iddbusWiimoteStatus = 0x06;
const uint16 iddbusWiimoteInfrared = 0x07;
const uint16 iddbusWiimoteAcc = 0x08;
const uint16 iddbusNunchukPlugged = 0x09;
const uint16 iddbusNunchukUnplugged = 0x0A;
const uint16 iddbusNunchukButtons = 0x0B;
const uint16 iddbusNunchukStick = 0x0C;
const uint16 iddbusNunchukAcc = 0x0D;
const uint16 iddbusClassicControllerPlugged = 0x0E;
const uint16 iddbusClassicControllerUnplugged = 0x0F;
const uint16 iddbusClassicControllerButtons = 0x10;
const uint16 iddbusClassicControllerLStick = 0x11;
const uint16 iddbusClassicControllerRStick = 0x12;

const uint16 iddbusResponseBool = 0xF0;
const uint16 iddbusResponseInt8 = 0xF1;

const uint16 iddbusWiimoteGetRumbleStatus = 0x01;
const uint16 iddbusWiimoteSetLedStatus = 0x02;
const uint16 iddbusWiimoteSetRumbleStatus = 0x03;
const uint16 iddbusWiimoteGetLedStatus = 0x04;

#endif //WIIMOTEDEVPROTO_H
