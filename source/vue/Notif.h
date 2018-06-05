/*
 * Notif.h
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_NOTIF_H_
#define SOURCE_VUE_NOTIF_H_

#include <list>
#include <stdint.h>
#include "WString.h"

typedef enum {
	eNotificationTypePartial,
	eNotificationTypeComplete,
} eNotificationType;

class Notif {
public:
	Notif(const char *title_, const char *msg_, uint8_t persist = 5, eNotificationType type_ = eNotificationTypePartial) {
		m_persist = persist;
		m_type    = type_;
		m_title   = title_;
		m_msg     = msg_;
	}

	uint8_t m_persist;
	eNotificationType m_type;
	String m_title;
	String m_msg;
};


class NotifiableDevice {
public:
	NotifiableDevice() {}
	~NotifiableDevice() {
		m_notifs.clear();
	}

	void addNotif(const char *title_, const char *msg_, uint8_t persist_ = 5, eNotificationType type_ = eNotificationTypePartial) {
		if (m_notifs.size() < 10) m_notifs.push_back(Notif(title_, msg_, persist_, type_));
	}

protected:
	std::list<Notif> m_notifs;
};

#endif /* SOURCE_VUE_NOTIF_H_ */
