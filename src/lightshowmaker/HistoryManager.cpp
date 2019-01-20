#include <utility>

#include "HistoryManager.h"
#include "Application.h"
#include "Serialization.h"
#include <chrono>

using namespace sm;

void HistoryManager::init(std::shared_ptr<model::Project> _proj) {
    proj = std::move(_proj);
    historyPos = 0;
    checkpoints.resize(0);
    inCommand = false;
    isMerging = false;
    asyncCommands.resize(0);
#ifndef NDEBUG
    currentStage = takeSnapshot();
#endif
}

bool HistoryManager::beginCommand(const std::string &name, bool mergeable) {
    if (!proj) return false;
    if (isMerging && (!mergeable || mergingCommand != name)) stopMerging();

    if (inCommand) {
        // Unended command
        assert(false);
        return false;
    }

    // clear redo history
    if (historyPos < checkpoints.size()) {
        checkpoints.erase(checkpoints.begin() + historyPos, checkpoints.end());
    }
    assert(historyPos == checkpoints.size());

    if (isMerging) {
        assert(historyPos > 0);
        assert(historyPos == checkpoints.size());
        // nothing special to do

    } else {
#ifndef NDEBUG
        if (currentStage != takeSnapshot()) {
            printf("Warning: project changed outside command\n");
        }
#endif

        // serialize state
        HistoryCheckpoint cmd;
        cmd.name = name;
        cmd.preState = snapshotGui(); // can change id of objects
        cmd.state = takeSnapshot();
        checkpoints.push_back(cmd);
        historyPos = checkpoints.size();

    }

    mergingCommand = name;
    inCommand = true;
    isMerging = mergeable;
    return true;
}

void HistoryManager::endCommand() {
    assert(inCommand);
    assert(historyPos > 0);
    assert(historyPos == checkpoints.size());
    inCommand = false;
    checkpoints[historyPos - 1].postState = snapshotGui();

#ifndef NDEBUG
    if (!isMerging) {
        currentStage = takeSnapshot();
    }
#endif
}

void HistoryManager::stopMerging() {
    assert(!inCommand);
    if (isMerging) {
        isMerging = false;
    }
}

void HistoryManager::asyncCommand(const std::string &name, bool mergeable, const std::function<void()> &fn) {
    asyncCommands.push_back(AppCommand{name, mergeable, fn});
}

void HistoryManager::update() {
    for (auto &cmd : asyncCommands) {
        if (beginCommand(cmd.name, cmd.mergeable)) {
            cmd.fn();
            endCommand();
        }
    }
    asyncCommands.resize(0);
}

void HistoryManager::undo() {
    PROFILE_BLOCK("HistoryManager::undo", 0.03);
    if (!canUndo()) return;

    if (historyPos == checkpoints.size()) {
        HistoryCheckpoint cmd;
        cmd.state = takeSnapshot();
        checkpoints.push_back(cmd);
    }

    historyPos--;
    unrollState(checkpoints[historyPos].state);
    unrollGui(checkpoints[historyPos].preState);
}

void HistoryManager::redo() {
    PROFILE_BLOCK("HistoryManager::redo", 0.03);
    if (!canRedo()) return;

    historyPos++;
    unrollState(checkpoints[historyPos].state);
    unrollGui(checkpoints[historyPos - 1].preState);
}

std::string HistoryManager::takeSnapshot() const {
#ifndef NDEBUG
    auto start_time = std::chrono::_V2::system_clock::now();
#endif

    std::string output = serializeObject(proj, "", true);
    size_t size = output.size();

#ifndef NDEBUG
    auto time = std::chrono::_V2::system_clock::now() - start_time;
    std::cout << "SNAPSHOT. Size:  " << size << "; time: " << time / std::chrono::milliseconds(1) << "ms\n";
#endif

    return output;
}

void HistoryManager::unrollState(std::string state) {
#ifndef NDEBUG
    auto start_time = std::chrono::_V2::system_clock::now();
#endif

    Serializer<DESER_JSON> serializer(state);
    serializer.setUndoRedo(true);
    proj->serialize(serializer);

#ifndef NDEBUG
    currentStage = state;
    auto time = std::chrono::_V2::system_clock::now() - start_time;
    std::cout << "SNAPSHOT restored. time: " << time / std::chrono::milliseconds(1) << "ms\n";
#endif
}

std::string HistoryManager::snapshotGui() const {
    Serializer<SER_JSON> serializer;
    serializer.setUndoRedo(true);
    serializer.serialize("window", gApp->getWindow());
    serializer.serialize("selection", gApp->getSelection());
    return serializer.jsonValue.toStyledString();
}

void HistoryManager::unrollGui(std::string guiState) {
    Serializer<DESER_JSON> serializer(guiState);
    serializer.setUndoRedo(true);
    serializer.serialize("window", gApp->getWindow());
    serializer.serialize("selection", gApp->getSelection());
}
