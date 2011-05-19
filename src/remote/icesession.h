#ifndef ICESESSION_H
#define ICESESSION_H

#include <pj/addr_resolv.h>
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjnath.h>

#include <QHostAddress>
#include <QList>
#include <QIODevice>
#include <QtDebug>

#include <xrme/connection.h>


class ICESession : public QIODevice {
  Q_OBJECT
 public:
  enum Direction {
    DirectionControlling = 0,
    DirectionControlled,
  };

  explicit ICESession(QObject* parent = 0);

  static void StaticInit();
  bool Init(Direction direction);

  const xrme::SIPInfo& candidates() const { return candidates_; }

  void StartNegotiation(const xrme::SIPInfo& session);

 protected:
  virtual qint64 readData(char* data, qint64 max_size);
  virtual qint64 writeData(const char* data, qint64 max_size);

 signals:
  void CandidatesAvailable(const xrme::SIPInfo& candidates);
  void Connected();

 private:
  pj_ice_strans* ice_instance_;
  int component_id_;

  xrme::SIPInfo candidates_;

  QByteArray receive_buffer_;

  void InitialisationComplete(pj_status_t status);

  static void PJLog(int level, const char* data, int len);

  static int HandleEvents(unsigned max_msec, unsigned* p_count);
  static int WorkerThread(void*);

  static void OnReceiveData(pj_ice_strans* ice_st,
                            unsigned comp_id,
                            void* pkt,
                            pj_size_t size,
                            const pj_sockaddr_t* src_addr,
                            unsigned src_addr_len);
  static void OnICEComplete(pj_ice_strans* ice_st,
                            pj_ice_strans_op op,
                            pj_status_t status);

  static pj_pool_t* sPool;
  static pj_caching_pool sCachingPool;
  static pj_ice_strans_cfg sIceConfig;
  static pj_thread_t* sThread;

  static int sComponentId;

  static const char* kStunServer;
};

QDebug operator<< (QDebug dbg, const xrme::SIPInfo& session);

#endif  // ICESESSION_H
