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
    template<typename T>
    void bulkEditor(const char *name, SelectionManager &manager,
                    std::function<T &(KeyRef)> getter,
                    std::function<T (KeyRef)> maxGetter);

    project::Fade fade;
    bool showSlider(const char *name, int max, int &item) const;
    bool showSlider(const char *name, float max, float &item) const;
};

}
}

#endif //PROPERTYPANEL_H
