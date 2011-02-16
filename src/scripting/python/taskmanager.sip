class TaskManager : QObject {

%TypeHeaderCode
#include "core/taskmanager.h"
%End

%Docstring
API for managing progress notifications in Clementine's status bar.

When you are performing a slow operation, such as downloading data from the
network or parsing a file, it is useful to be able to provide progress
information to the user.  Clementine displays information about any running
tasks with a spinner animation in the status bar.

When you start a new task, the TaskManager will give you an ID which you must
keep and use again later to update the task's progress or to mark it as
completed.

This is a singleton class, you can access its methods directly through the
C{clementine.task_manager} object:

  >>> id = clementine.task_manager.StartTask("Finding some awesome music")
  >>> clementine.task_manager.SetTaskProgress(id, 33, 100)
  >>> clementine.task_manager.SetTaskProgress(id, 66, 100)
  >>> clementine.task_manager.SetTaskFinished(id)

@group Signals: TasksChanged, PauseLibraryWatchers, ResumeLibraryWatchers
%End

public:
  struct Task {
%Docstring
Object containing information about a running task.
%End

    int id;
    QString name;
    int progress;
    int progress_max;
    bool blocks_library_scans;
  };

  QList<TaskManager::Task> GetTasks();
%Docstring
GetTasks() -> list of L{Task}s
Returns a list of all the tasks that are currently running.

Modifying the items in this list will have no effect - you have to use the other
methods in this class to update the status of a task.
%End

  int StartTask(const QString& name);
%Docstring
StartTask(name) -> int
Begins a new task.

@param name: A descriptive name of the task that will be displayed in the
  status bar.
@type name: str
@return: A new ID for this task.  Use this ID when calling L{SetTaskFinished}
  later.
@note: Do not include ellipsis (...) in the name of the task, this is added
  for you by the status bar.
%End

  void SetTaskBlocksLibraryScans(int id);
%Docstring
SetTaskBlocksLibraryScans(id)
Forces any library scans to wait until this task is complete.

It may be useful to set this flag on a task that modifies files in the library.
The LibraryWatcher will wait until the task is complete before starting to
rescan the Library.
%End

  void SetTaskProgress(int id, int progress, int max = 0);
%Docstring
SetTaskProgress(id, progress, max=0)
Updates the progress for this task.

The status bar shows the percentage progress of this task, calculated by
C{(progress / max * 100)}.  C{max} must be provided the first time this
method is called for a given task, but after that it can be omitted to keep the
old value.
%End

  void SetTaskFinished(int id);
%Docstring
SetTaskFinished(id)
Marks the given task as finished, removing it from the status bar.
%End

signals:
  void TasksChanged();
%Docstring
TasksChanged()
Emitted when the list of tasks returned by L{GetTasks()} is changed.
%End

  void PauseLibraryWatchers();
%Docstring
PauseLibraryWatchers()
Emitted when L{SetTaskBlocksLibraryScans()} is called on a task.
%End

  void ResumeLibraryWatchers();
%Docstring
ResumeLibraryWatchers()
Emitted when all tasks that blocked library scans have finished.
%End

private:
  TaskManager();
};
