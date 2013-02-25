/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef LOCKABLE_EXCEPTION_H
#define LOCKABLE_EXCEPTION_H

#include <exception>
#include <string>

class LockableException : public std::exception {
private:
	std::string message;
	bool locked;
public:
	LockableException() {
		message = "";
		locked = false;
	}

	LockableException(std::string msg) {
		message = msg;
		locked = false;
	}

	~LockableException() throw() { }

	void appendMessage(std::string msg) {
		if (!locked) message.append(msg);
	}

	void appendMessageAndLock(std::string msg) {
		appendMessage(msg);
		locked = true;
	}

	void lock() {
		locked = true;
	}

	const char *what() const throw() {
		return message.c_str();
	}
};

#endif
