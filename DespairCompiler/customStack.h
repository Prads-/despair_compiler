/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef CUSTOM_STACK_H
#define CUSTOM_STACK_H

template <typename Type>
class Stack {
private:
	Type *mem;
	unsigned int size, counter;
	void checkBoundary() {
		if (counter == size) {
			const int INCRESE_BY = 50;

			Type *newMem = new Type[size + INCRESE_BY];
			for (unsigned int i = 0; i < size; ++i) {
				newMem[i] = mem[i];
			}
			delete [] mem;
			mem = newMem;
			size += INCRESE_BY;
		}
	}
public:
	Stack() {
		counter = 0;
		size = 100;
		mem = new Type[size];
	}

	~Stack() {
		delete [] mem;
	}

	void push(Type val) {
		checkBoundary();
		mem[counter++] = val;
	}

	Type pop() {
		return mem[counter--];
	}

	//This won't delete the last item, might be handy sometimes
	Type getLastItem() {
		return mem[counter - 1];
	}

	void clear() {
		counter = 0;
	}
};

#endif