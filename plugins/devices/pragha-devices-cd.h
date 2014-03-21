/*************************************************************************/
/* Copyright (C) 2012-2014 matias <mati86dl@gmail.com>                   */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_LIBCDIO && HAVE_LIBCDIO_PARANOIA && HAVE_LIBCDDB

#ifndef PRAGHA_DEVICES_CD_H
#define PRAGHA_DEVICES_CD_H

#include "pragha-devices-plugin.h"

void pragha_devices_audio_cd_added (PraghaDevicesPlugin *plugin);

#endif /* PRAGHA_DEVICES_CD_H */

#endif
