#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include "core.h"
#include "cassert"
#include "vector"
#include "string"
#include "functional"
#include "memory"
#include "Project.h"

namespace sm {


class HistoryManager {
public:

    void init(std::shared_ptr<model::Project> proj);
    bool beginCommand(const std::string &name, bool mergeable = false);
    void endCommand();
    void stopMerging();
    void asyncCommand(const std::string &name, bool mergeable, const std::function<void()> &fn);
    void update();

    bool canUndo() { return historyPos > 0; }
    bool canRedo() { return !checkpoints.empty() && historyPos < checkpoints.size() - 1; }

    std::string undoMsg() { return canUndo() ? checkpoints[historyPos - 1].name : ""; }
    std::string redoMsg() { return canRedo() ? checkpoints[historyPos].name : ""; }

    void undo();
    void redo();

private:
    std::string takeSnapshot() const;
    std::string snapshotGui() const;

    struct AppCommand {
        std::string name;
        bool mergeable;
        std::function<void()> fn;
    };

    struct HistoryCheckpoint {
        std::string name;
        std::string state;
        std::string preState;
        std::string postState;
    };

    std::shared_ptr<model::Project> proj;
    std::vector<HistoryCheckpoint> checkpoints;
    std::vector<AppCommand> asyncCommands;

    // indicate the current status index
    size_t historyPos = 0;

    bool inCommand = false;
    bool isMerging = false;
    std::string mergingCommand;
#ifndef NDEBUG
    std::string currentStage;
#endif
    void unrollState(std::string state);
    void unrollGui(std::string guiState);
};

}

#endif //HISTORYMANAGER_H
