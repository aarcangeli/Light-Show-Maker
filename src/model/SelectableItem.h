#ifndef SELECTABLEITEM_H
#define SELECTABLEITEM_H

#include "core.h"
#include "Serialization.h"

namespace sm {
namespace model {

template<typename SELF>
class SelectableItem {
public:
    bool isSelected = false;

    // serialized only in undo/redo snapshots
    int64_t id = -1;

    SERIALIZATION_START_DECL(serializeBase) {
        if (ser.isUndoRedo()) {
            ser.serialize("id", id);
        }
    };
};

}
}

#endif //SELECTABLEITEM_H
