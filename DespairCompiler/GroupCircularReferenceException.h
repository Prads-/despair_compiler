#ifndef GROUP_CIRCULAR_REFERENCE_EXCEPTION_H
#define GROUP_CIRCULAR_REFERENCE_EXCEPTION_H

#include <exception>
#include <string>

class GroupCircularReferenceException : public std::exception {
private:
	std::string message;
public:
	GroupCircularReferenceException(std::string msg) {
		message = msg;
	}

	void appendMessage(std::string msg) {
		message.append(msg);
	}

	const char *what() const throw() {
		return message.c_str();
	}
};

#endif