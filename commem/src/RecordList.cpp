#include "RecordList.h"


void RecordList::add(const byte_t* record, int size) {
	list.push_back(Record(record,size));
}

void RecordList::clear() {
	list.clear();
}

int RecordList::size() const {
	return list.size();
}

const byte_t* RecordList::get(int pos) const {
	return list[pos].data;
}

