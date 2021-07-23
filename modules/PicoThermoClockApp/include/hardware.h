/*!
 * \file
 * \brief Hardware configuration file
 * \author Dawid Babula
 * \email dbabula@adventurous.pl
 *
 * \par Copyright (C) Dawid Babula, 2018
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef HARDWARE_H
#define HARDWARE_H

/*!
 *
 * \defgroup MiniThermometer
 * \addtogroup hardware
 * \ingroup MiniThermometer
 * \brief Configures which PCB pin assignment shall be used
 */

/*@{*/

#if defined PCB
#if (PCB == 1U)
    #include "../source/PCB0001.h"
#else
    #error "Choose version of PCB to which project shall apply"
#endif
#else
    #error "PCB is not defined"
#endif

/*@}*/
#endif /* end of HARDWARE_H */
