#ifndef __RECORD_LIST_H__
#define __RECORD_LIST_H__
#include "CommonMemTable.h"


	class RecordList {
		private:
			struct Record {
				byte_t data[STRING_MAX_SIZE];
				int length;
				Record(const byte_t* buf, int size) {
					length = size;
					memcpy(data, buf, size);
				}
				Record(const Record& x) {
					length = x.length;
					memcpy(data, x.data, sizeof(data));
				}
				Record& operator=(const Record& x) {
					length = x.length;
					memcpy(data, x.data, sizeof(data));
					return *this;
				}
			};
			vector<Record> list;

		public:
			void add(const byte_t* record, int size);
			void clear();
			int size() const;
			const byte_t* get(int pos) const;
	};


#endif
