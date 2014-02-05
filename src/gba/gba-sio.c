#include "gba-sio.h"

#include "gba-io.h"

#include <limits.h>

static struct GBASIODriver* _lookupDriver(struct GBASIO* sio, enum GBASIOMode mode) {
	switch (mode) {
	case SIO_MULTI:
		return sio->drivers.multiplayer;
	case SIO_JOYBUS:
		return sio->drivers.joybus;
	default:
		return 0;
	}
}

static void _switchMode(struct GBASIO* sio) {
	int mode = ((sio->rcnt >> 14) & 0xC) | ((sio->siocnt >> 12) & 0x3);
	enum GBASIOMode oldMode = sio->mode;
	if (mode < 8) {
		sio->mode = (enum GBASIOMode) (mode & 0x3);
	} else {
		sio->mode = (enum GBASIOMode) (mode & 0xC);
	}
	if (oldMode != mode) {
		if (sio->activeDriver && sio->activeDriver->unload) {
			sio->activeDriver->unload(sio->activeDriver);
		}
		sio->activeDriver = _lookupDriver(sio, mode);
		if (sio->activeDriver && sio->activeDriver->load) {
			sio->activeDriver->load(sio->activeDriver);
		}
	}
}

void GBASIOInit(struct GBASIO* sio) {
	sio->rcnt = RCNT_INITIAL;
	sio->siocnt = 0;
	_switchMode(sio);
}

void GBASIODeinit(struct GBASIO* sio) {
	if (sio->drivers.multiplayer && sio->drivers.multiplayer->deinit) {
		sio->drivers.multiplayer->deinit(sio->drivers.multiplayer);
	}
	if (sio->drivers.joybus && sio->drivers.joybus->deinit) {
		sio->drivers.joybus->deinit(sio->drivers.joybus);
	}
}

void GBASIOSetDriverSet(struct GBASIO* sio, struct GBASIODriverSet* drivers) {
	if (drivers->multiplayer) {
		GBASIOSetDriver(sio, drivers->multiplayer, SIO_MULTI);
	}
	if (drivers->joybus) {
		GBASIOSetDriver(sio, drivers->multiplayer, SIO_JOYBUS);
	}
}

void GBASIOSetDriver(struct GBASIO* sio, struct GBASIODriver* driver, enum GBASIOMode mode) {
	struct GBASIODriver** driverLoc;
	switch (mode) {
	case SIO_MULTI:
		driverLoc = &sio->drivers.multiplayer;
		break;
	case SIO_JOYBUS:
		driverLoc = &sio->drivers.joybus;
		break;
	default:
		GBALog(sio->p, GBA_LOG_ERROR, "Setting an unsupported SIO driver: %x", mode);
		return;
	}
	if (*driverLoc) {
		if ((*driverLoc)->unload) {
			(*driverLoc)->unload(*driverLoc);
		}
		if ((*driverLoc)->deinit) {
			(*driverLoc)->deinit(*driverLoc);
		}
	}
	if (driver) {
		driver->p = sio;

		if (driver->init) {
			if (!driver->init(driver)) {
				driver->deinit(driver);
				GBALog(sio->p, GBA_LOG_ERROR, "Could not initialize SIO driver");
				return;
			}
		}
		if (*driverLoc && *driverLoc == sio->activeDriver) {
			sio->activeDriver = driver;
			if ((*driverLoc)->load) {
				(*driverLoc)->load(*driverLoc);
			}
		}
	}
	*driverLoc = driver;
}

void GBASIOWriteRCNT(struct GBASIO* sio, uint16_t value) {
	sio->rcnt = value;
	_switchMode(sio);
	if (sio->activeDriver && sio->activeDriver->writeRegister) {
		sio->activeDriver->writeRegister(sio->activeDriver, REG_RCNT, value);
	}
}

void GBASIOWriteSIOCNT(struct GBASIO* sio, uint16_t value) {
	sio->siocnt = value;
	_switchMode(sio);
	if (sio->activeDriver && sio->activeDriver->writeRegister) {
		sio->activeDriver->writeRegister(sio->activeDriver, REG_SIOCNT, value);
	}
}

void GBASIOWriteSIOMLT_SEND(struct GBASIO* sio, uint16_t value) {
	if (sio->activeDriver && sio->activeDriver->writeRegister) {
		sio->activeDriver->writeRegister(sio->activeDriver, REG_SIOMLT_SEND, value);
	}
}

int32_t GBASIOProcessEvents(struct GBASIO* sio, int32_t cycles) {
	if (sio->activeDriver && sio->activeDriver->processEvents) {
		return sio->activeDriver->processEvents(sio->activeDriver, cycles);
	}
	return INT_MAX;
}