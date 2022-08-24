/*
 * map-request-record.h
 *
 *  Created on: Apr 4, 2017
 *      Author: qsong
 */

#ifndef SRC_INTERNET_MODEL_LISP_CONTROL_PLANE_MAP_REQUEST_RECORD_H_
#define SRC_INTERNET_MODEL_LISP_CONTROL_PLANE_MAP_REQUEST_RECORD_H_

#include "ns3/address.h"
#include "ns3/lisp-control-msg.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"



namespace ns3 {

class MapRequestRecord : public SimpleRefCount<MapRequestRecord>
{
public:
  MapRequestRecord ();
  MapRequestRecord (Address eidPrefix, uint8_t eidMaskLength);
  virtual
  ~MapRequestRecord ();

  /*
   * \returns Value of the Notification Requested bit.
   *
   * Get the value of the Notification Requested bit. see draft-ietf-lisp-pubsub
   */
  uint8_t GetN (void) const;
  /*
   * \param n Value of the Notification Requested bit.
   *
   * Set the value of the Notification Requested bit. see draft-ietf-lisp-pubsub
   */
  void SetN (uint8_t n);

  LispControlMsg::AddressFamily GetAfi (void);
  void SetAfi (LispControlMsg::AddressFamily afi);

  void SetMaskLenght (uint8_t maskLenght);
  uint8_t GetMaskLength (void);

  void SetEidPrefix (Address prefix);
  Address GetEidPrefix (void);

  /*
   * \returns The size of the serialized record in bytes.
   *
   * Get the size of the records once serialized (in bytes).
   */
  uint8_t GetSizeInBytes (void) const;

  void Serialize (uint8_t *buf) const;
  static Ptr<MapRequestRecord> Deserialize (uint8_t *buf);

  void Print (std::ostream& os);
private:
  // Notification Requested bit, see draft-ietf-lisp-pubsub.
  uint8_t m_N : 1;
  LispControlMsg::AddressFamily m_afi;
  uint8_t m_eidMaskLenght;
  Address m_eidPrefix;
};

} /* namespace ns3 */

#endif /* SRC_INTERNET_MODEL_LISP_CONTROL_PLANE_MAP_REQUEST_RECORD_H_ */
