#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include "Selection.h"
#include "functional"

namespace sm {
namespace editor {

class PropertyPanel {
    using KeyRef = std::shared_ptr<project::KeyPoint>;

public:
    void showProperties(SelectionManager &manager);
    void showPropertiesOf(std::shared_ptr<project::Layer> &layer);
    void showPropertiesOf(std::shared_ptr<project::Decoration> &layer);
    void showPropertiesOf(std::shared_ptr<project::KeyPoint> &keypoint);
    void showPropertiesOf(project::Fade &keypoint, const char *name, time_unit max);

    void multiKeypointEditor(SelectionManager &manager);

private:
    void bulkEditor(const char *name, SelectionManager &manager,
                    std::function<int &(KeyRef)> getter,
                    std::function<int (KeyRef)> maxGetter);

    project::Fade fade;
};

}
}

#endif //PROPERTYPANEL_H
