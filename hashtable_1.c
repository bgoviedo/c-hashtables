#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable_1.h"
#include "HashTables/lockfreetable.h"
#include "HashTables/lockingtable.h"
#include "HashTables/probetable.h"

hashtable_1* new_hashtable_1(ht_type type, int capacity) {
  switch (type) {
    case LOCKING:
//        printf("Locking\n");
      return (hashtable_1*) new_lockingtable(capacity);
    case LINEARPROBE:
//        printf("Linearprobe\n");
      return (hashtable_1*)new_probetable(capacity * 4);
    default:  // LOCKFREE
//        printf("Lockfree\n");
      return (hashtable_1*)new_lockfreetable(capacity);
  }
}
