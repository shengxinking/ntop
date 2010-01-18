/*
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *
 *                          http://www.ntop.org
 *
 *             Copyright (C) 2010 Luca Deri <deri@ntop.org>
 *                                Daniele Sgandurra <sgandurra@ntop.org>
 *                                Jaime Blasco <jaime.blasco@alienvault.com>
 *                                Gianluca Medici <medici@ntop.org>
 *
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include "ntop.h"
#include "globals-report.h"

#ifdef HAVE_PYTHON

static HostTraffic *ntop_host = NULL;
static char query_string[2048];
static pthread_mutex_t python_mutex;

/* **************************************** */

static PyObject* python_sendHTTPHeader(PyObject *self, PyObject *args) {
  int mime_type;

  // traceEvent(CONST_TRACE_WARNING, "-%s-", "python_sendHTTPHeader");

  if(!PyArg_ParseTuple(args, "i", &mime_type)) return NULL;

  sendHTTPHeader(mime_type /* FLAG_HTTP_TYPE_HTML */, 0, 0);
  return PyString_FromString("");
}

/* **************************************** */

static PyObject* python_printHTMLHeader(PyObject *self,
				       PyObject *args) {
  char *title;
  int sectionTitle, refresh;
  int flags = 0;

  // traceEvent(CONST_TRACE_WARNING, "-%s-", "python_printHTMLHeader");

  if(!PyArg_ParseTuple(args, "sii", &title,
		       &sectionTitle, &refresh)) return NULL;


  if(sectionTitle == 0) flags |= BITFLAG_HTML_NO_HEADING;
  if(refresh == 0)      flags |= BITFLAG_HTML_NO_REFRESH;

  printHTMLheader(title, NULL, flags);
  return PyString_FromString("");
}

/* **************************************** */

static PyObject* python_printHTMLFooter(PyObject *self,
				       PyObject *args) {

  // traceEvent(CONST_TRACE_WARNING, "-%s-", "python_printHTMLFooter");

  printHTMLtrailer();
  return PyString_FromString("");
}

/* **************************************** */

static PyObject* python_sendString(PyObject *self,
				   PyObject *args) {
  char *msg;

  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_sendString");

  /* parse the incoming arguments */
  if (!PyArg_ParseTuple(args, "s", &msg)) {
      return NULL;
    }

  sendString(msg);
  return PyString_FromString("");
}

/* **************************************** */

static PyObject* python_printFlagedWarning(PyObject *self,
					   PyObject *args) {
  char *msg;

  /* parse the incoming arguments */
  if (!PyArg_ParseTuple(args, "s", &msg)) {
      return NULL;
    }
  
  printFlagedWarning(msg);
  return PyString_FromString("");
}

/* **************************************** */

static PyObject* python_getFirstHost(PyObject *self,
				     PyObject *args) {
  int actualDeviceId;

  // traceEvent(CONST_TRACE_WARNING, "-%s- [%p]", "python_getFirstHost", ntop_host);

  /* parse the incoming arguments */
  if(!PyArg_ParseTuple(args, "i", &actualDeviceId))
    return NULL;

  ntop_host = getFirstHost(actualDeviceId);

  //Return PyString_FromString(ntop_host ? "1" : "0");
  return Py_BuildValue("i", ntop_host ? 1 : 0);
}

/* **************************************** */

static PyObject* python_findHostByNumIP(PyObject *self,
					PyObject *args) {
  char *hostIpAddress;
  int vlanId;
  int actualDeviceId;
  HostAddr addr;

  // traceEvent(CONST_TRACE_WARNING, "-%s- [%p]", "python_findHostByNumIP", ntop_host);

  /* parse the incoming arguments */
  if(!PyArg_ParseTuple(args, "sii", &hostIpAddress, &vlanId, &actualDeviceId))
    return NULL;

  addr.Ip4Address.s_addr = inet_addr(hostIpAddress); /* FIX: add IPv6 support */
  ntop_host = findHostByNumIP(addr, vlanId, actualDeviceId);

  return Py_BuildValue("i", ntop_host ? 1 : 0);
}

/* **************************************** */

static PyObject* python_getPreference(PyObject *self,
				      PyObject *args) {
  char *key, value[512] = { '\0' };
  int rc;

  if(!PyArg_ParseTuple(args, "s", &key)) return NULL;

  rc = fetchPrefsValue(key, value, sizeof(value));
  return PyString_FromString(rc == 0 ? value : "");
}

/* **************************************** */

static PyObject* python_getDBPath(PyObject *self, PyObject *args) {
  return PyString_FromString(myGlobals.dbPath);
}

static PyObject* python_getSpoolPath(PyObject *self, PyObject *args) {
  return PyString_FromString(myGlobals.spoolPath);
}

/* **************************************** */

static PyObject* python_setPreference(PyObject *self,
				      PyObject *args) {
  char *key, *value;

  if(!PyArg_ParseTuple(args, "ss", &key, &value)) return NULL;

  storePrefsValue(key, value);
  return PyInt_FromLong(0);
}

/* **************************************** */

static PyObject* python_getNextHost(PyObject *self,
				    PyObject *args) {
  int actualDeviceId;

  //traceEvent(CONST_TRACE_WARNING, "-%s- [%p]", "python_getNextHost", ntop_host);

  /* parse the incoming arguments */
  if(!PyArg_ParseTuple(args, "i", &actualDeviceId))
    return NULL;

  if(ntop_host != NULL)
    ntop_host = getNextHost(actualDeviceId, ntop_host);
  else
    ntop_host = getFirstHost(actualDeviceId);

  //return PyString_FromString(ntop_host ? "1" : "0");
  return Py_BuildValue("i", ntop_host ? 1 : 0);

}

/* **************************************** */

static PyObject* python_hostSerial(PyObject *self,
				   PyObject *args) {
  char buf[64];

  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_hostSerial");
  return PyString_FromString(ntop_host ? serial2str(ntop_host->hostSerial, buf, sizeof(buf)) : "");
}

/* **************************************** */

static PyObject* python_ethAddress(PyObject *self,
				   PyObject *args) {

  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ethAddress");
  return PyString_FromString(ntop_host ? ntop_host->ethAddressString : "");
}

/* **************************************** */

static PyObject* python_ipAddress(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyString_FromString((ntop_host && ntop_host->hostNumIpAddress) ? ntop_host->hostNumIpAddress : "");
}


/* **************************************** */

static PyObject* python_hostResolvedName(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyString_FromString((ntop_host && ntop_host->hostResolvedName) ? ntop_host->hostResolvedName : "");
}

/* **************************************** */

static PyObject* python_hostTrafficBucket(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyLong_FromUnsignedLong(ntop_host->hostTrafficBucket);
}

/* **************************************** */

static PyObject* python_numHostSessions(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyLong_FromUnsignedLong(ntop_host->numHostSessions);
}

/* **************************************** */

static PyObject* python_vlanId(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyLong_FromUnsignedLong(ntop_host->vlanId);
}

/* **************************************** */

static PyObject* python_networkMask(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyLong_FromUnsignedLong(ntop_host->network_mask);
}

/* **************************************** */

static PyObject* python_hwModel(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyString_FromString((ntop_host && ntop_host->hwModel) ? ntop_host->hwModel : "");
}

/* **************************************** */

static PyObject* python_isFTPhost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isFTPhost(ntop_host));
}

/* **************************************** */

static PyObject* python_isServer(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isServer(ntop_host));
}

/* **************************************** */

static PyObject* python_isWorkstation(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isWorkstation(ntop_host));
}

/* **************************************** */

static PyObject* python_isMasterBrowser(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isMasterBrowser(ntop_host));
}

/* **************************************** */

static PyObject* python_isMultihomed(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isMultihomed(ntop_host));
}

/* **************************************** */

static PyObject* python_isMultivlaned(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isMultivlaned(ntop_host));
}

/* **************************************** */

static PyObject* python_isPrinter(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isPrinter(ntop_host));
}

/* **************************************** */

static PyObject* python_isSMTPhost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isSMTPhost(ntop_host));
}

/* **************************************** */

static PyObject* python_isPOPhost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isPOPhost(ntop_host));
}

/* **************************************** */

static PyObject* python_isIMAPhost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isIMAPhost(ntop_host));
}

/* **************************************** */

static PyObject* python_isDirectoryHost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isDirectoryHost(ntop_host));
}

/* **************************************** */

static PyObject* python_isHTTPhost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isHTTPhost(ntop_host));
}

/* **************************************** */

static PyObject* python_isWINShost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isWINShost(ntop_host));
}

/* **************************************** */

static PyObject* python_isBridgeHost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isBridgeHost(ntop_host));
}

/* **************************************** */

static PyObject* python_isVoIPClient(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isVoIPClient(ntop_host));
}

/* **************************************** */

static PyObject* python_isVoIPGateway(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isVoIPGateway(ntop_host));
}

/* **************************************** */

static PyObject* python_isVoIPHost(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isVoIPHost(ntop_host));
}

/* **************************************** */

static PyObject* python_isDHCPClient(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isDHCPClient(ntop_host));
}

/* **************************************** */

static PyObject* python_isDHCPServer(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isDHCPServer(ntop_host));
}

/* **************************************** */

static PyObject* python_isP2P(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isP2P(ntop_host));
}

/* **************************************** */

static PyObject* python_isNtpServer(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyBool_FromLong(isNtpServer(ntop_host));
}

/* **************************************** */

static PyObject* python_totContactedSentPeers(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyLong_FromUnsignedLong((unsigned long)(unsigned long)(ntop_host->totContactedSentPeers));
}

/* **************************************** */

static PyObject* python_totContactedRcvdPeers(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyLong_FromUnsignedLong((unsigned long)(unsigned long)(ntop_host->totContactedRcvdPeers));
}

/* **************************************** */

static PyObject* python_fingerprint(PyObject *self, PyObject *args) {
  return PyString_FromString((ntop_host && ntop_host->fingerprint) ? ntop_host->fingerprint : "");
}

/* **************************************** */

static PyObject* python_pktsSent(PyObject *self, PyObject *args) {
  return PyLong_FromUnsignedLong((unsigned long)(unsigned long)(ntop_host->pktSent.value));
}

static PyObject* python_pktsRcvd(PyObject *self, PyObject *args) {
  return PyLong_FromUnsignedLong((unsigned long)(unsigned long)(ntop_host->pktRcvd.value));
}

/* **************************************** */

static PyObject* python_bytesSent(PyObject *self, PyObject *args) {
  return PyLong_FromUnsignedLong((unsigned long)(unsigned long)(ntop_host->bytesSent.value));
}

static PyObject* python_bytesRcvd(PyObject *self, PyObject *args) {
  return PyLong_FromUnsignedLong((unsigned long)(unsigned long)(ntop_host->bytesRcvd.value));
}

/* **************************************** */

#ifdef HAVE_GEOIP

#define VAL(a) ((a != NULL) ? a : "")

static PyObject* python_getGeoIP(PyObject *self, PyObject *args) {
  PyObject *obj = PyDict_New();
  GeoIPRecord *geo = (ntop_host && ntop_host->geo_ip) ? ntop_host->geo_ip : NULL;

  if(geo != NULL) {
    PyDict_SetItem(obj, PyString_FromString("country_code"), PyString_FromString(VAL(geo->country_code)));
    PyDict_SetItem(obj, PyString_FromString("country_name"), PyString_FromString(VAL(geo->country_name)));
    PyDict_SetItem(obj, PyString_FromString("region"), PyString_FromString(VAL(geo->region)));
    PyDict_SetItem(obj, PyString_FromString("city"), PyString_FromString(VAL(geo->city)));
    PyDict_SetItem(obj, PyString_FromString("latitude"), PyFloat_FromDouble((double)geo->latitude));
    PyDict_SetItem(obj, PyString_FromString("longitude"), PyFloat_FromDouble((double)geo->longitude));
  }

  return obj;
}
#endif

/* **************************************** */

static PyObject* python_synPktsSent(PyObject *self,
				   PyObject *args) {
  //traceEvent(CONST_TRACE_WARNING, "-%s-", "python_ipAddress");

  return PyString_FromFormat("%lu",
			     (ntop_host && ntop_host->secHostPkts) ?
			     (unsigned long)(ntop_host->secHostPkts->synPktsSent.value.value) : 0);
}

/* **************************************** */

static PyObject* python_interface_numInterfaces(PyObject *self, PyObject *args) {
  return PyInt_FromLong((long)myGlobals.numDevices);
}

static PyObject* python_interface_name(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;  
  return PyString_FromFormat("%s", myGlobals.device[interfaceId].name);
}

static PyObject* python_interface_uniqueIfName(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
    return PyString_FromFormat("%s", myGlobals.device[interfaceId].uniqueIfName);
}

static PyObject* python_interface_humanFriendlyName(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  return PyString_FromFormat("%s", myGlobals.device[interfaceId].humanFriendlyName);
}

static PyObject* python_interface_ipv4Address(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  return PyString_FromString(myGlobals.device[interfaceId].ipdot ? myGlobals.device[interfaceId].ipdot : "");
}

static PyObject* python_interface_network(PyObject *self, PyObject *args) {
  u_int interfaceId;
  char buf[32], buf1[32];

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  return PyString_FromFormat("%s/%s", 
			     _intoa(myGlobals.device[interfaceId].network, buf, sizeof(buf)),
			     _intoa(myGlobals.device[interfaceId].netmask, buf1, sizeof(buf1))
			     );
}

static PyObject* python_interface_numHosts(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  
  return PyLong_FromUnsignedLong(myGlobals.device[interfaceId].numHosts);
}

#ifdef INET6
static PyObject* python_interface_ipv6Address(PyObject *self, PyObject *args) {
  u_int interfaceId;
  char buf[64];

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if(myGlobals.device[interfaceId].v6Addrs == NULL) return PyString_FromString("");

  return PyString_FromFormat("%s/%d", 
			     _intop(&myGlobals.device[interfaceId].v6Addrs->af.inet6.ifAddr, buf, sizeof(buf)),
			     myGlobals.device[interfaceId].v6Addrs->af.inet6.prefixlen);
}
#endif

static PyObject* python_interface_time(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("startTime"), PyLong_FromUnsignedLong((u_int)myGlobals.device[interfaceId].started));
  PyDict_SetItem(obj, PyString_FromString("firstSeen"), PyLong_FromUnsignedLong((u_int)myGlobals.device[interfaceId].firstpkt));
  PyDict_SetItem(obj, PyString_FromString("lastSeen"), PyLong_FromUnsignedLong((u_int)myGlobals.device[interfaceId].lastpkt));
  return obj;
}

static PyObject* python_interface_virtual(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  
  return PyInt_FromLong((long)myGlobals.device[interfaceId].virtualDevice);
}

static PyObject* python_interface_speed(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  
  return PyLong_FromUnsignedLong(myGlobals.device[interfaceId].deviceSpeed);
}

static PyObject* python_interface_mtu(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  
  return PyLong_FromUnsignedLong(myGlobals.device[interfaceId].mtuSize);
}

static PyObject* python_interface_bpf(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  
  return PyString_FromString(myGlobals.device[interfaceId].filter ? myGlobals.device[interfaceId].filter : "");
}

static PyObject* python_interface_pktsStats(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("total"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].receivedPkts.value));
  PyDict_SetItem(obj, PyString_FromString("ntopDrops"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].droppedPkts.value));
  PyDict_SetItem(obj, PyString_FromString("pcapDrops"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].pcapDroppedPkts.value));
  PyDict_SetItem(obj, PyString_FromString("initialPcapDrops"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].initialPcapDroppedPkts.value));
  PyDict_SetItem(obj, PyString_FromString("ethernet"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].ethernetPkts.value));
  PyDict_SetItem(obj, PyString_FromString("broadcast"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].broadcastPkts.value));
  PyDict_SetItem(obj, PyString_FromString("multicast"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].multicastPkts.value));
  PyDict_SetItem(obj, PyString_FromString("ip"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].ipPkts.value));

  return obj;
}

#ifdef ENABLE_FC

static PyObject* python_interface_fcPktsStats(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("total"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].fcPkts.value));
  PyDict_SetItem(obj, PyString_FromString("eofa"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].fcEofaPkts.value));
  PyDict_SetItem(obj, PyString_FromString("eofAbnormal"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].fcEofAbnormalPkts.value));
  PyDict_SetItem(obj, PyString_FromString("abnormal"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].fcAbnormalPkts.value));
  PyDict_SetItem(obj, PyString_FromString("broadcast"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].fcBroadcastPkts.value));
  return obj;
}

static PyObject* python_interface_fcBytesStats(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("total"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fcBytes.value));
  PyDict_SetItem(obj, PyString_FromString("fragmented"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fragmentedFcBytes.value));
  PyDict_SetItem(obj, PyString_FromString("fcp"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fcFcpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("ficon"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fcFiconBytes.value));
  PyDict_SetItem(obj, PyString_FromString("ip"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fcIpfcBytes.value));
  PyDict_SetItem(obj, PyString_FromString("swils"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fcSwilsBytes.value));
  PyDict_SetItem(obj, PyString_FromString("dns"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fcDnsBytes.value));
  PyDict_SetItem(obj, PyString_FromString("els"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fcElsBytes.value));
  PyDict_SetItem(obj, PyString_FromString("other"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].otherFcBytes.value));
  PyDict_SetItem(obj, PyString_FromString("broadcast"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].fcBroadcastBytes.value));
  PyDict_SetItem(obj, PyString_FromString("class2"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].class2Bytes.value));
  PyDict_SetItem(obj, PyString_FromString("class3"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].class3Bytes.value));
  PyDict_SetItem(obj, PyString_FromString("classFb"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].classFBytes.value));
  return obj;
}

#endif

static PyObject* python_interface_bytesStats(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("total"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].ethernetBytes.value));
  PyDict_SetItem(obj, PyString_FromString("ipv4"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].ipv4Bytes.value));
  PyDict_SetItem(obj, PyString_FromString("fragmented"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].fragmentedIpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("tcp"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].tcpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("udp"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].udpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("otherIp"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].otherIpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("icmp"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].icmpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("dlc"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].dlcBytes.value));
  PyDict_SetItem(obj, PyString_FromString("ipx"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].ipxBytes.value));
  PyDict_SetItem(obj, PyString_FromString("stp"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].stpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("ipsec"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].ipsecBytes.value));
  PyDict_SetItem(obj, PyString_FromString("netbios"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netbiosBytes.value));
  PyDict_SetItem(obj, PyString_FromString("arp_rarp"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].arpRarpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("appletalk"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].atalkBytes.value));
  PyDict_SetItem(obj, PyString_FromString("egp"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].egpBytes.value));
  PyDict_SetItem(obj, PyString_FromString("gre"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].greBytes.value));
  PyDict_SetItem(obj, PyString_FromString("ipv6"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].ipv6Bytes.value));
  PyDict_SetItem(obj, PyString_FromString("icmp6"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].icmp6Bytes.value));
  PyDict_SetItem(obj, PyString_FromString("other"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].otherBytes.value));
  return(obj);
}

static PyObject* python_interface_throughputStats(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("peakPkts"), PyFloat_FromDouble(myGlobals.device[interfaceId].peakPacketThroughput));
  PyDict_SetItem(obj, PyString_FromString("actualPkts"), PyFloat_FromDouble(myGlobals.device[interfaceId].actualPktsThpt));
  PyDict_SetItem(obj, PyString_FromString("lastMinPkts"), PyFloat_FromDouble(myGlobals.device[interfaceId].lastMinPktsThpt));
  PyDict_SetItem(obj, PyString_FromString("lastFiveMinsPkts"), PyFloat_FromDouble(myGlobals.device[interfaceId].lastFiveMinsPktsThpt));
  PyDict_SetItem(obj, PyString_FromString("peakBytes"), PyFloat_FromDouble(myGlobals.device[interfaceId].peakThroughput));
  PyDict_SetItem(obj, PyString_FromString("actualBytes"), PyFloat_FromDouble(myGlobals.device[interfaceId].actualPktsThpt));
  PyDict_SetItem(obj, PyString_FromString("lastMinBytes"), PyFloat_FromDouble(myGlobals.device[interfaceId].lastMinPktsThpt));
  PyDict_SetItem(obj, PyString_FromString("lastFiveMinsBytes"), PyFloat_FromDouble(myGlobals.device[interfaceId].lastFiveMinsPktsThpt));
  return(obj);
}

static PyObject* python_interface_SimpleProtoTrafficInfo(SimpleProtoTrafficInfo *info) {
  PyObject *obj;

  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("local"), PyLong_FromUnsignedLong((unsigned long)info->local.value));
  PyDict_SetItem(obj, PyString_FromString("local2remote"), PyLong_FromUnsignedLong((unsigned long)info->local2remote.value));
  PyDict_SetItem(obj, PyString_FromString("remote"), PyLong_FromUnsignedLong((unsigned long)info->remote.value));
  PyDict_SetItem(obj, PyString_FromString("remote2local"), PyLong_FromUnsignedLong((unsigned long)info->remote2local.value));

  return(obj);
}

static PyObject* python_interface_tcpStats(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  return(python_interface_SimpleProtoTrafficInfo(&myGlobals.device[interfaceId].tcpGlobalTrafficStats));
}

static PyObject* python_interface_udpStats(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  return(python_interface_SimpleProtoTrafficInfo(&myGlobals.device[interfaceId].udpGlobalTrafficStats));
}

static PyObject* python_interface_icmpStats(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  return(python_interface_SimpleProtoTrafficInfo(&myGlobals.device[interfaceId].icmpGlobalTrafficStats));
}

static PyObject* python_interface_ipStats(PyObject *self, PyObject *args) {
  u_int interfaceId;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if(myGlobals.device[interfaceId].ipProtoStats == NULL) return NULL;
  return(python_interface_SimpleProtoTrafficInfo(myGlobals.device[interfaceId].ipProtoStats));
}

static PyObject* python_interface_securityPkts(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("synPkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.synPkts.value));
  PyDict_SetItem(obj, PyString_FromString("rstPkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.rstPkts.value));
  PyDict_SetItem(obj, PyString_FromString("rstAckPkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.rstAckPkts.value));
  PyDict_SetItem(obj, PyString_FromString("synFinPkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.synFinPkts.value));
  PyDict_SetItem(obj, PyString_FromString("finPushUrgPkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.finPushUrgPkts.value));
  PyDict_SetItem(obj, PyString_FromString("nullPkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.nullPkts.value));
  
  PyDict_SetItem(obj, PyString_FromString("rejectedTCPConn"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.rejectedTCPConn.value));
  PyDict_SetItem(obj, PyString_FromString("establishedTCPConn"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.establishedTCPConn.value));
  PyDict_SetItem(obj, PyString_FromString("terminatedTCPConn"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.terminatedTCPConn.value));
  PyDict_SetItem(obj, PyString_FromString("ackXmasFinSynNullScan"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.ackXmasFinSynNullScan.value));
  PyDict_SetItem(obj, PyString_FromString("udpToClosedPort"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.udpToClosedPort.value));
  PyDict_SetItem(obj, PyString_FromString("udpToDiagnosticPort"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.udpToDiagnosticPort.value));
  PyDict_SetItem(obj, PyString_FromString("tcpToDiagnosticPort"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.tcpToDiagnosticPort.value));
  PyDict_SetItem(obj, PyString_FromString("tinyFragment"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.tinyFragment.value));
  PyDict_SetItem(obj, PyString_FromString("icmpFragment"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.icmpFragment.value));
  PyDict_SetItem(obj, PyString_FromString("overlappingFragment"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.overlappingFragment.value));
  PyDict_SetItem(obj, PyString_FromString("closedEmptyTCPConn"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.closedEmptyTCPConn.value));
  PyDict_SetItem(obj, PyString_FromString("malformedPkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.malformedPkts.value));
  PyDict_SetItem(obj, PyString_FromString("icmpPortUnreach"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.icmpPortUnreach.value));
  PyDict_SetItem(obj, PyString_FromString("icmpHostNetUnreach"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.icmpHostNetUnreach.value));
  PyDict_SetItem(obj, PyString_FromString("icmpProtocolUnreach"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.icmpProtocolUnreach.value));
  PyDict_SetItem(obj, PyString_FromString("icmpAdminProhibited"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].securityPkts.icmpAdminProhibited.value));

  return(obj);
}

static PyObject* python_interface_netflowStats(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if(myGlobals.device[interfaceId].netflowGlobals == NULL) return PyDict_New();
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("totalPkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsPktsRcvd));
  PyDict_SetItem(obj, PyString_FromString("v5flows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsV5Rcvd));
  PyDict_SetItem(obj, PyString_FromString("v1flows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsV1Rcvd));
  PyDict_SetItem(obj, PyString_FromString("v7flows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsV7Rcvd));
  PyDict_SetItem(obj, PyString_FromString("v9flows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsV9Rcvd));
  PyDict_SetItem(obj, PyString_FromString("flowsProcessed"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsProcessed));
  PyDict_SetItem(obj, PyString_FromString("flowsReceived"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsRcvd));
  PyDict_SetItem(obj, PyString_FromString("badVersion"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numBadNetFlowsVersionsRcvd));
  PyDict_SetItem(obj, PyString_FromString("badFlows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numBadFlowPkts));
  PyDict_SetItem(obj, PyString_FromString("v9Templates"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsV9TemplRcvd));
  PyDict_SetItem(obj, PyString_FromString("v9BadTemplates"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsV9BadTemplRcvd));
  PyDict_SetItem(obj, PyString_FromString("v9UnknownTemplate"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsV9UnknTemplRcvd));
  PyDict_SetItem(obj, PyString_FromString("v9OptionFlows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].netflowGlobals->numNetFlowsV9OptionFlowsRcvd));

  return(obj);
}

static PyObject* python_interface_sflowStats(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if(myGlobals.device[interfaceId].sflowGlobals == NULL) return PyDict_New();
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("pkts"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].sflowGlobals->numsFlowsPktsRcvd));
  PyDict_SetItem(obj, PyString_FromString("v2Flows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].sflowGlobals->numsFlowsV2Rcvd));
  PyDict_SetItem(obj, PyString_FromString("v4Flows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].sflowGlobals->numsFlowsV4Rcvd));
  PyDict_SetItem(obj, PyString_FromString("v5Flows"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].sflowGlobals->numsFlowsV5Rcvd));
  PyDict_SetItem(obj, PyString_FromString("totalProcessed"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].sflowGlobals->numsFlowsProcessed));
  PyDict_SetItem(obj, PyString_FromString("samples"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].sflowGlobals->numsFlowsSamples));
  PyDict_SetItem(obj, PyString_FromString("counterUpdates"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].sflowGlobals->numsFlowCounterUpdates));
  PyDict_SetItem(obj, PyString_FromString("badVersion"), PyLong_FromUnsignedLong((unsigned long)myGlobals.device[interfaceId].sflowGlobals->numBadsFlowsVersionsRcvd));

  return(obj);
}


static PyObject* python_interface_cpacketStats(PyObject *self, PyObject *args) {
  u_int interfaceId;
  PyObject *obj;

  if(!PyArg_ParseTuple(args, "i", &interfaceId)) return NULL;
  if(interfaceId >= myGlobals.numDevices) return NULL;
  if(myGlobals.device[interfaceId].cpacketGlobals == NULL) return PyDict_New();
  if((obj = PyDict_New()) == NULL) return NULL;

  PyDict_SetItem(obj, PyString_FromString("total"), PyLong_FromUnsignedLong(myGlobals.device[interfaceId].cpacketGlobals->statsProcessed));

  return(obj);
}

/* **************************************** */

static PyMethodDef ntop_methods[] = {
  { "sendHTTPHeader", python_sendHTTPHeader, METH_VARARGS| METH_KEYWORDS, "" },
  { "printHTMLHeader", python_printHTMLHeader, METH_VARARGS, "" },

  { "printHTMLFooter", python_printHTMLFooter, METH_VARARGS, "" },
  { "sendString",      python_sendString,      METH_VARARGS, "" },
  { "printFlagedWarning",      python_printFlagedWarning,      METH_VARARGS, "" },

  { "getFirstHost",    python_getFirstHost,    METH_VARARGS, "" },
  { "getNextHost",     python_getNextHost,     METH_VARARGS, "" },
  { "findHostByNumIP", python_findHostByNumIP, METH_VARARGS, "" },

  { "getPreference",      python_getPreference,      METH_VARARGS, "" },
  { "setPreference",      python_setPreference,      METH_VARARGS, "" },

  { "getDBPath",         python_getDBPath,      METH_VARARGS, "" },
  { "getSpoolPath",      python_getSpoolPath,   METH_VARARGS, "" },
  { NULL, NULL, 0, NULL }
};

/* **************************************** */

static PyMethodDef interface_methods[] = {
  { "numInterfaces", python_interface_numInterfaces, METH_VARARGS, "Get number of configured interfaces" },
  { "name", python_interface_name, METH_VARARGS, "Get interface name" },
  { "uniqueName", python_interface_uniqueIfName, METH_VARARGS, "Get unique interface name" },
  { "humanName", python_interface_humanFriendlyName, METH_VARARGS, "Get human-friendly interface name" },
  { "ipv4", python_interface_ipv4Address, METH_VARARGS, "Get interface address (IPv4)" },
  { "network", python_interface_network, METH_VARARGS, "Get network and mask to which the interface belongs" },
  { "numHosts", python_interface_numHosts, METH_VARARGS, "Get the number of hosts active on this interface" },
#ifdef INET6
  { "ipv6", python_interface_ipv6Address, METH_VARARGS, "Get interface address (IPv6)" },
#endif
  { "time", python_interface_time, METH_VARARGS, "Get interface time" },
  { "virtual", python_interface_virtual, METH_VARARGS, "Check if this is a virtual interface" },
  { "speed", python_interface_speed, METH_VARARGS, "Interface speed (0 if unknown)" },
  { "mtu", python_interface_mtu, METH_VARARGS, "Get interface MTU size" },
  { "bpf", python_interface_bpf, METH_VARARGS, "Get BPF filter set for this interface (if any)" },
  { "pktsStats", python_interface_pktsStats, METH_VARARGS, "Get packet statistics " },
#ifdef ENABLE_FC
  { "fcPktsStats", python_interface_fcPktsStats, METH_VARARGS, "Get FC pkts stats" },
  { "fcBytesStats", python_interface_fcBytesStats, METH_VARARGS, "Get FC byte stats" },
#endif
  { "bytesStats", python_interface_bytesStats, METH_VARARGS, "Get bytes statistics" },
  { "throughputStats", python_interface_throughputStats, METH_VARARGS, "" },
  { "tcpStats", python_interface_tcpStats, METH_VARARGS, "Get TCP stats" },
  { "udpStats", python_interface_udpStats, METH_VARARGS, "Get UDP stats" },
  { "icmpStats", python_interface_icmpStats, METH_VARARGS, "Get ICMP stats" },
  { "ipStats", python_interface_ipStats, METH_VARARGS, "Get IP stats" },
  { "securityPkts", python_interface_securityPkts, METH_VARARGS, "Get information about security packets" },
  { "netflowStats", python_interface_netflowStats, METH_VARARGS, "Get NetFlow interface information" },
  { "sflowStats", python_interface_sflowStats, METH_VARARGS, "Get sFlow interface information" },
  { "cpacketStats", python_interface_cpacketStats, METH_VARARGS, "Get cPacket counter information" },
  { NULL, NULL, 0, NULL }
};

/* **************************************** */

static PyMethodDef host_methods[] = {
  { "serial", python_hostSerial, METH_NOARGS, "Get host unique serial identifier" },
  { "ethAddress", python_ethAddress, METH_NOARGS, "Get host MAC address" },
  { "ipAddress",  python_ipAddress, METH_NOARGS, "Get host IP address" },
  { "hostResolvedName",  python_hostResolvedName, METH_NOARGS, "Get host Resolved Name" },
  { "hostTrafficBucket",  python_hostTrafficBucket, METH_NOARGS, "Get Traffic Bucket" },
  { "numHostSessions",  python_numHostSessions, METH_NOARGS, "Get numHostSessions" },
  { "vlanId",  python_vlanId, METH_NOARGS, "Get vlanId" },
  { "network_mask",  python_networkMask, METH_NOARGS, "Get network_mask" },
  { "hwModel",  python_hwModel, METH_NOARGS, "Get hwModel" },
  { "isFTPhost",  python_isFTPhost, METH_NOARGS, "Check FTP Host" },
  { "isServer",  python_isServer, METH_NOARGS, "Check isServer" },
  { "isWorkstation",  python_isWorkstation, METH_NOARGS, "Check isWorkstation Host" },
  { "isMasterBrowser",  python_isMasterBrowser, METH_NOARGS, "Check isMasterBrowser Host" },
  { "isMultihomed",  python_isMultihomed, METH_NOARGS, "Check isMultihomed Host" },
  { "isMultivlaned",  python_isMultivlaned, METH_NOARGS, "Check isMultivlaned Host" },
  { "isPrinter",  python_isPrinter, METH_NOARGS, "Check isPrinter Host" },
  { "isSMTPhost",  python_isSMTPhost, METH_NOARGS, "Check isSMTPhost Host" },
  { "isPOPhost",  python_isPOPhost, METH_NOARGS, "Check isPOPhost Host" },
  { "isIMAPhost",  python_isIMAPhost, METH_NOARGS, "Check isIMAPhost Host" },
  { "isDirectoryHost",  python_isDirectoryHost, METH_NOARGS, "Check isDirectoryHost Host" },
  { "isHTTPhost",  python_isHTTPhost, METH_NOARGS, "Check isHTTPhost Host" },
  { "isWINShost",  python_isWINShost, METH_NOARGS, "Check isWINShost Host" },
  { "isBridgeHost",  python_isBridgeHost, METH_NOARGS, "Check isBridgeHost Host" },
  { "isVoIPClient",  python_isVoIPClient, METH_NOARGS, "Check isVoIPClient Host" },
  { "isVoIPGateway",  python_isVoIPGateway, METH_NOARGS, "Check isVoIPGateway Host" },
  { "isVoIPHost",  python_isVoIPHost, METH_NOARGS, "Check isVoIPHost Host" },
  { "isDHCPClient",  python_isDHCPClient, METH_NOARGS, "Check isDHCPClient Host" },
  { "isDHCPServer",  python_isDHCPServer, METH_NOARGS, "Check isDHCPServer Host" },
  { "isP2P",  python_isP2P, METH_NOARGS, "Check isP2P Host" },
  { "isNtpServer",  python_isNtpServer, METH_NOARGS, "Check isNtpServer Host" },
  { "totContactedSentPeers",  python_totContactedSentPeers, METH_NOARGS, "Check totContactedSentPeers Host" },
  { "totContactedRcvdPeers",  python_totContactedRcvdPeers, METH_NOARGS, "Check totContactedRcvdPeers Host" },
  { "fingerprint",  python_fingerprint, METH_NOARGS, "Check fingerprint Host" },
  { "synPktsSent",  python_synPktsSent, METH_NOARGS, "Check synPktsSent Host" },
  { "pktSent",  python_pktsSent, METH_NOARGS, "Return the number of packets sent by this host" },
  { "pktRcvd",  python_pktsRcvd, METH_NOARGS, "Return the number of packets rcvd by this host" },
  { "bytesSent",  python_bytesSent, METH_NOARGS, "Return the number of bytes sent by this host" },
  { "bytesRcvd",  python_bytesRcvd, METH_NOARGS, "Return the number of bytes rcvd by this host" },
#ifdef HAVE_GEOIP
  { "geoIP",  python_getGeoIP, METH_NOARGS, "Read geoLocalization info" },
#endif
  { NULL, NULL, 0, NULL }
};

/* **************************************** */

static void init_python_ntop(void) {
  pthread_mutex_init(&python_mutex, 0);
  Py_InitModule("ntop", ntop_methods);
  Py_InitModule("host", host_methods);
  Py_InitModule("interface", interface_methods);
}

/* **************************************** */
int _argc = 0;
char **_argv;

void init_python(int argc, char *argv[]) {

  if(_argc == 0) {
    _argc = argc;
    _argv = argv;
  }

  if(argv) Py_SetProgramName(argv[0]);

  /* Initialize the Python interpreter.  Required. */
  Py_Initialize();

  if(argv) PySys_SetArgv(argc, argv);

  /* Initialize thread support */
  PyEval_InitThreads();

  init_python_ntop();
}

/* **************************************** */

void term_python(void) {
  Py_Finalize();   /* Cleaning up the interpreter */
}

/* **************************************** */

int handlePythonHTTPRequest(char *url, u_int postLen) {
  int idx, found = 0;
  char python_path[256], *document_root = ".", buf[2048];
  struct stat statbuf;
  FILE *fd;
  char *question_mark = strchr(url, '?');

  // traceEvent(CONST_TRACE_INFO, "Calling python... [%s]", url);

  if(question_mark) question_mark[0] = '\0';
  safe_snprintf(__FILE__, __LINE__, query_string, sizeof(query_string)-1,
		"%s", question_mark ? &question_mark[1] : "");

  for(idx=0; myGlobals.dataFileDirs[idx] != NULL; idx++) {
    char tmpStr[256];

    safe_snprintf(__FILE__, __LINE__, tmpStr, sizeof(tmpStr),
		  "%s/html", myGlobals.dataFileDirs[idx]);
    revertSlashIfWIN32(tmpStr, 0);
    if(stat(tmpStr, &statbuf) == 0) {
      document_root = myGlobals.dataFileDirs[idx];
      break;
    }
  }

  for(idx=0; (!found) && (myGlobals.dataFileDirs[idx] != NULL); idx++) {
    safe_snprintf(__FILE__, __LINE__, python_path, sizeof(python_path),
		  "%s/python/%s", myGlobals.dataFileDirs[idx], url);
    revertSlashIfWIN32(python_path, 0);

    if(!stat(python_path, &statbuf)) {
      /* Found */
      /* traceEvent(CONST_TRACE_INFO, "[python] [%d] Found %s", idx, python_path); */
      found = 1;
      break;
    } else {
      /* traceEvent(CONST_TRACE_INFO, "[python] [%d] Not found %s", idx, python_path); */
    }
  }

  if(!found) {
    returnHTTPpageNotFound(NULL);
    return(1);
  }

  /* ********************************* */

  /* traceEvent(CONST_TRACE_INFO, "[PYTHON] Executing %s", python_path); */

  if((fd = fopen(python_path, "r")) != NULL) {
    int old_stdin = dup(STDIN_FILENO), old_stdout = dup(STDOUT_FILENO);

    /* TODO: remove this mutex */
    pthread_mutex_lock(&python_mutex);

   if(postLen == 0) {
     /* HTTP GET */
     safe_snprintf(__FILE__, __LINE__, buf, sizeof(buf),
		   "import os\nos.environ['DOCUMENT_ROOT']='%s'\n"
		   "os.environ['REQUEST_METHOD']='GET'\n"
		   "os.environ['QUERY_STRING']='%s'\n",
		   document_root, query_string);
   } else {
     /* HTTP POST */
     safe_snprintf(__FILE__, __LINE__, buf, sizeof(buf),
		   "import os\nos.environ['DOCUMENT_ROOT']='%s'\n"
		   "os.environ['REQUEST_METHOD']='POST'\n"
		   "os.environ['CONTENT_LENGTH']='%u'\n",
		   document_root, postLen);
   }

   /* See http://bugs.python.org/issue1159 */
    PyRun_SimpleString(buf);

    /* sys.stdin <=> myGlobals.newSock */

    if(dup2(myGlobals.newSock, STDOUT_FILENO) == -1)
      traceEvent(CONST_TRACE_WARNING, "Failed to redirect stdout");

    if(dup2(myGlobals.newSock, STDIN_FILENO) == -1)
      traceEvent(CONST_TRACE_WARNING, "Failed to redirect stdin");
   
    /* Run the actual program */
    PyRun_SimpleFile(fd, python_path);
    
    if(dup2(old_stdin, STDOUT_FILENO) == -1)
      traceEvent(CONST_TRACE_WARNING, "Failed to restore stdout");

    if(dup2(old_stdout, STDIN_FILENO) == -1)
      traceEvent(CONST_TRACE_WARNING, "Failed to restore stdout");

    pthread_mutex_unlock(&python_mutex);
  }

  fclose(fd);

  return(1);
}


#endif /* HAVE_PYTHON */