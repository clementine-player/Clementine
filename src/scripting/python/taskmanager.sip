class TaskManager : QObject {

%TypeHeaderCode
#include "core/taskmanager.h"
%End

public:
  struct Task {
    int id;
    QString name;
    int progress;
    int progress_max;
    bool blocks_library_scans;
  };

  QList<TaskManager::Task> GetTasks();

  int StartTask(const QString& name);
  void SetTaskBlocksLibraryScans(int id);
  void SetTaskProgress(int id, int progress, int max = 0);
  void SetTaskFinished(int id);

signals:
  void TasksChanged();

  void PauseLibraryWatchers();
  void ResumeLibraryWatchers();

private:
  TaskManager();
};
