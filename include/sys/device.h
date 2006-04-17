/*
 * Copyright 2006 by Brian Dominy <brian@oddchange.com>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _SYS_DEVICE_H
#define _SYS_DEVICE_H

/** The maximum number of switches that a ball device can have */
#ifndef MAX_SWITCHES_PER_DEVICE
#define MAX_SWITCHES_PER_DEVICE 6
#endif

/** The maximum number of ball devices that are supported */
#ifndef MAX_DEVICES
#define MAX_DEVICES 8
#endif

/** The GIDs for the device update tasks.
 * Every device has its own GID for the function that runs to service it.
 * This task is in charge of processing switch closures AND servicing
 * kick requests.
 */
#define DEVICE_GID_BASE		GID_DEVICE0_ACTIVE
#define DEVICE_GID1			GID_DEVICE1_ACTIVE
#define DEVICE_GID2			GID_DEVICE2_ACTIVE
#define DEVICE_GID3			GID_DEVICE3_ACTIVE
#define DEVICE_GID4			GID_DEVICE4_ACTIVE
#define DEVICE_GID5			GID_DEVICE5_ACTIVE
#define DEVICE_GID6			GID_DEVICE6_ACTIVE
#define DEVICE_GID7			GID_DEVICE7_ACTIVE


/** Translate a device number (devno), which is zero-based, into
 * its corresponding GID. */
#define DEVICE_GID(devno)	(GID_DEVICE0_ACTIVE + (devno))


struct device;

/** The device operations structure.  Each device can define callbacks
 * for various types of events/operations that are device-specific.
 * You can leave a field as NULL and it will not be called.
 * These are game-specific functions, as all of the basic stuff is done
 * by the common code here. */
typedef struct device_ops
{
	/** Called when the machine is powered up */
	void (*power_up) (struct device *dev);

	/** Called whenever a game is started */
	void (*game_start) (struct device *dev);

	/** Called whenever a ball enters the device */
	void (*enter) (struct device *dev);

	/** Called whenever the game tries to kick a ball from the device */
	void (*kick_attempt) (struct device *dev);

	/** Called when a kick is successful */
	void (*kick_success) (struct device *dev);

	/** Called when a kick is not successful */
	void (*kick_failure) (struct device *dev);

	/** Called when the device becomes full */
	void (*full) (struct device *dev);

	/** Called when the device becomes empty */
	void (*empty) (struct device *dev);
} device_ops_t;


/** Macro used by the device module to invoke callback operations.
 * If an operation isn't defined, then it is skipped, so you need
 * not provide all of the operations if you don't need them all.
 * These hooks are only called during an actual game; the system
 * is fully in charge during test/attract mode.
 */
#define device_call_op(dev, op) \
do { \
	dbprintf ("Calling device hook %s\n", #op); \
	if (in_live_game && (dev->props->ops->op)) \
	{ \
		(*dev->props->ops->op) (dev); \
	} \
} while (0)


/** The device structure is a read-only descriptor that
 * contains various device properties. */
typedef struct device_properties
{
	/** Pointer to the operations structure */
	device_ops_t *ops;

	/** Name of the device */
	const char *name;

	/** The solenoid used to kick balls from it */
	solnum_t sol;

	/** The number of switches in the device for counting balls */
	uint8_t sw_count;

	/** The initial number of switches that ought to be closed;
	 * i.e. the number of balls that the device is allowed to hold
	 * at initialization time.  Any extra balls found will be kicked. */
	uint8_t init_max_count;

	/** The switch numbers for the switches; switch 0 always refers to
	 * the first switch that would close when a ball enters the device,
	 * switch MAX-1 is the last switch to open when a ball leaves it.  */
	switchnum_t sw[MAX_SWITCHES_PER_DEVICE];
} device_properties_t;


/**
 * Device states.  Each device is governed by a state
 * machine; these values dictate the various states that
 * a device can be in.
 */
#define DEV_STATE_IDLE			0
#define DEV_STATE_ENTERED		1
#define DEV_STATE_RELEASING	2

/** The device info structure.  This is a read-write
 * structure that maintains the current state of a device.
 * Included is the state machine state, as well as other
 * properties like how many balls are currently in the
 * device. */
typedef struct device
{
	/** Device number assigned to this device */
	uint8_t devno;

	/** The size of the device, same as the number of counting switches */
	uint8_t size;

	/** The current count of balls */
	uint8_t actual_count;

	/** The previous count of balls */
	uint8_t previous_count;

	/** The maximum number of balls that can be held here. */
	uint8_t max_count;

	/** The number of balls needed to be kicked out */
	uint8_t kicks_needed;

	/** The operational state of the device */
	uint8_t state;

	/** Pointer to the device property structure */
	device_properties_t *props;
} device_t;

typedef uint8_t devicenum_t;

#define device_entry(devno)	(&device_table[devno])
#define device_devno(dev)		(dev->devno)

#define device_empty_p(dev)	(dev->actual_count == 0)
#define device_full_p(dev)		(dev->actual_count == dev->max_count)

#define device_disable_lock(dev)	(dev->max_count--)
#define device_enable_lock(dev)	(dev->max_count++)

extern device_t device_table[];

void device_clear (device_t *dev);
void device_register (devicenum_t devno, device_properties_t *props);
uint8_t device_recount (device_t *dev);
void device_update_globals (void);
void device_probe (void);
void device_request_kick (device_t *dev);
void device_request_empty (device_t *dev);
void device_sw_handler (uint8_t devno);
void device_add_live (void);
void device_remove_live (void);
void device_multiball_set (U8 count);
void device_init (void);

void trough_init (void);

#endif /* _SYS_DEVICE_H */
