/*
 *  Copyright (C) 1998-2000 Luca Deri <deri@ntop.org>
 *                          Portions by Stefano Suin <stefano@ntop.org>
 *
 *  			  Centro SERRA, University of Pisa
 *  			  http://www.ntop.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "ntop.h"


/* ******************************* */

void updateThptStats(int deviceToUpdate,
		     int topSentIdx, int secondSentIdx, int thirdSentIdx,
		     int topHourSentIdx, int secondHourSentIdx,
		     int thirdHourSentIdx,
		     int topRcvdIdx, int secondRcvdIdx, int thirdRcvdIdx,
		     int topHourRcvdIdx, int secondHourRcvdIdx,
		     int thirdHourRcvdIdx) 
{
  int i;

#ifdef DEBUG
  traceEvent(TRACE_INFO, "updateThptStats(%d, %d, %d, %d, %d, %d)\n",
	 topSentIdx, secondSentIdx, thirdSentIdx,
	 topHourSentIdx, secondHourSentIdx,
	 thirdHourSentIdx);
#endif

  /* We're never check enough... */
  if(topSentIdx == -1) 
    return;

  if(topRcvdIdx == -1) 
    return;

  if(secondSentIdx == -1) 
    secondSentIdx = 0;

  if(thirdSentIdx == -1)
    thirdSentIdx = 0;

  if(secondRcvdIdx == -1)
    secondRcvdIdx = 0;

  if(thirdRcvdIdx == -1)
    thirdRcvdIdx = 0;

  for(i=58; i>=0; i--)
    memcpy(&device[deviceToUpdate].last60MinutesThpt[i+1],
	   &device[deviceToUpdate].last60MinutesThpt[i], sizeof(ThptEntry));

  device[deviceToUpdate].last60MinutesThpt[0].trafficValue = device[deviceToUpdate].lastMinThpt;

  device[deviceToUpdate].last60MinutesThpt[0].topHostSentIdx = topSentIdx,
    device[deviceToUpdate].last60MinutesThpt[0].topSentTraffic = device[deviceToUpdate].hash_hostTraffic[topSentIdx]->actualSentThpt;
  device[deviceToUpdate].last60MinutesThpt[0].secondHostSentIdx = secondSentIdx,
    device[deviceToUpdate].last60MinutesThpt[0].secondSentTraffic = device[deviceToUpdate].hash_hostTraffic[secondSentIdx]->actualSentThpt;
  device[deviceToUpdate].last60MinutesThpt[0].thirdHostSentIdx = thirdSentIdx,
    device[deviceToUpdate].last60MinutesThpt[0].thirdSentTraffic = device[deviceToUpdate].hash_hostTraffic[thirdSentIdx]->actualSentThpt;

  device[deviceToUpdate].last60MinutesThpt[0].topHostRcvdIdx = topRcvdIdx,
    device[deviceToUpdate].last60MinutesThpt[0].topRcvdTraffic = device[deviceToUpdate].hash_hostTraffic[topRcvdIdx]->actualRcvdThpt;
  device[deviceToUpdate].last60MinutesThpt[0].secondHostRcvdIdx = secondRcvdIdx,
    device[deviceToUpdate].last60MinutesThpt[0].secondRcvdTraffic = device[deviceToUpdate].hash_hostTraffic[secondRcvdIdx]->actualRcvdThpt;
  device[deviceToUpdate].last60MinutesThpt[0].thirdHostRcvdIdx = thirdRcvdIdx,
    device[deviceToUpdate].last60MinutesThpt[0].thirdRcvdTraffic = device[deviceToUpdate].hash_hostTraffic[thirdRcvdIdx]->actualRcvdThpt;

  device[deviceToUpdate].last60MinutesThptIdx = (device[deviceToUpdate].last60MinutesThptIdx+1) % 60;

  if(topHourSentIdx != -1) { /* It wrapped -> 1 hour is over */
    float average=0;
    int i;

    if(topHourSentIdx == -1) return;
    if(topHourRcvdIdx == -1) return;
    if(secondHourSentIdx == -1) secondHourSentIdx = 0;
    if(thirdHourSentIdx == -1)  thirdHourSentIdx = 0;
    if(secondHourRcvdIdx == -1) secondHourRcvdIdx = 0;
    if(thirdHourRcvdIdx == -1)  thirdHourRcvdIdx = 0;

    for(i=0; i<60; i++) {
      average += device[deviceToUpdate].last60MinutesThpt[i].trafficValue;
    }

    average /= 60;

    for(i=22; i>=0; i--)
      memcpy(&device[deviceToUpdate].last24HoursThpt[i+1], 
	     &device[deviceToUpdate].last24HoursThpt[i], sizeof(ThptEntry));

    device[deviceToUpdate].last24HoursThpt[0].trafficValue = average;

    device[deviceToUpdate].last24HoursThpt[0].topHostSentIdx = topHourSentIdx,
      device[deviceToUpdate].last24HoursThpt[0].topSentTraffic = device[deviceToUpdate].hash_hostTraffic[topHourSentIdx]->lastHourSentThpt;
    device[deviceToUpdate].last24HoursThpt[0].secondHostSentIdx = secondHourSentIdx,
      device[deviceToUpdate].last24HoursThpt[0].secondSentTraffic = device[deviceToUpdate].hash_hostTraffic[secondHourSentIdx]->lastHourSentThpt;
    device[deviceToUpdate].last24HoursThpt[0].thirdHostSentIdx = thirdHourSentIdx,
      device[deviceToUpdate].last24HoursThpt[0].thirdSentTraffic = device[deviceToUpdate].hash_hostTraffic[thirdHourSentIdx]->lastHourSentThpt;

    device[deviceToUpdate].last24HoursThpt[0].topHostRcvdIdx = topHourRcvdIdx,
      device[deviceToUpdate].last24HoursThpt[0].topRcvdTraffic = device[deviceToUpdate].hash_hostTraffic[topHourRcvdIdx]->lastHourRcvdThpt;
    device[deviceToUpdate].last24HoursThpt[0].secondHostRcvdIdx = secondHourRcvdIdx,
      device[deviceToUpdate].last24HoursThpt[0].secondRcvdTraffic = device[deviceToUpdate].hash_hostTraffic[secondHourRcvdIdx]->lastHourRcvdThpt;
    device[deviceToUpdate].last24HoursThpt[0].thirdHostRcvdIdx = thirdHourRcvdIdx,
      device[deviceToUpdate].last24HoursThpt[0].thirdRcvdTraffic = device[deviceToUpdate].hash_hostTraffic[thirdHourRcvdIdx]->lastHourRcvdThpt;

    device[deviceToUpdate].last24HoursThptIdx = (device[deviceToUpdate].last24HoursThptIdx + 1) % 24;

    if(device[deviceToUpdate].last24HoursThptIdx == 0) {
      average=0;

      for(i=0; i<24; i++) {
	average += device[deviceToUpdate].last24HoursThpt[i].trafficValue;
      }

      average /= 24;

      for(i=28; i>=0; i--) {
	device[deviceToUpdate].last30daysThpt[i+1] = device[deviceToUpdate].last30daysThpt[i];
      }

      device[deviceToUpdate].last30daysThpt[0] = average;
      device[deviceToUpdate].last30daysThptIdx = (device[deviceToUpdate].last30daysThptIdx + 1) % 30;
    }
  }

  device[deviceToUpdate].numThptSamples++;
  

#ifdef DEBUG
  traceEvent(TRACE_INFO, "updateThptStats() completed.\n");
#endif
}

/* ******************************* */

void updateDeviceThpt(int deviceToUpdate) {
  time_t timeDiff, timeHourDiff=0, totalTime;
  u_int idx;
  HostTraffic *el;

  timeDiff = actTime-device[deviceToUpdate].lastThptUpdate;

  /* traceEvent(TRACE_INFO, "%u %u %u\n", timeDiff, throughput, ethernetBytes); */

  if(timeDiff > 5 /* secs */) {
    int topSentIdx=-1, secondSentIdx=-1, thirdSentIdx=-1;\
    int topHourSentIdx=-1, secondHourSentIdx=-1, thirdHourSentIdx=-1;
    int topRcvdIdx=-1, secondRcvdIdx=-1, thirdRcvdIdx=-1;\
    int topHourRcvdIdx=-1, secondHourRcvdIdx=-1, thirdHourRcvdIdx=-1;
    short updateMinThpt, updateHourThpt;

    totalTime = actTime-initialSniffTime;

    updateHourThpt = 0;
    updateMinThpt = 0;

    if((timeDiff = actTime-device[deviceToUpdate].lastMinThptUpdate) >= 60 /* 1 minute */) {
      updateMinThpt = 1;
      device[deviceToUpdate].lastMinThptUpdate = actTime;
      if((timeHourDiff = actTime-device[deviceToUpdate].lastHourThptUpdate) >= 60*60 /* 1 hour */) {
	updateHourThpt = 1;
	device[deviceToUpdate].lastHourThptUpdate = actTime;
      }
    }

    for(idx=1; idx<device[actualDeviceId].actualHashSize; idx++) {
      if((el = device[deviceToUpdate].hash_hostTraffic[idx]) != NULL) {

	if(broadcastHost(el))
	  continue;

	el->actualRcvdThpt    = ((float)(el->bytesReceived-el->lastBytesReceived)/timeDiff)/* *8 */;
	if(el->peakRcvdThpt   < el->actualRcvdThpt) el->peakRcvdThpt = el->actualRcvdThpt;
	if(el->peakSentThpt   < el->actualSentThpt) el->peakSentThpt = el->actualSentThpt;
	el->actualSentThpt    = ((float)(el->bytesSent-el->lastBytesSent)/timeDiff)/* *8 */;
	el->lastBytesSent     = el->bytesSent;
	el->lastBytesReceived = el->bytesReceived;

	/* ******************************** */

	el->actualRcvdPktThpt  = (float)(el->pktReceived-el->lastPktReceived)/timeDiff;
	if(el->peakRcvdPktThpt < el->actualRcvdPktThpt) el->peakRcvdPktThpt = el->actualRcvdPktThpt;
	if(el->peakSentPktThpt < el->actualSentPktThpt) el->peakSentPktThpt = el->actualSentPktThpt;
	el->actualSentPktThpt  = (float)(el->pktSent-el->lastPktSent)/timeDiff;
	el->lastPktSent        = el->pktSent;
	el->lastPktReceived    = el->pktReceived;

	/* ******************************** */

	if(updateMinThpt) {
	  el->averageRcvdThpt = (((float)el->bytesReceived)/totalTime)/* *8 */;
	  el->averageSentThpt = (((float)el->bytesSent)/totalTime)/* *8 */;
	  el->averageRcvdPktThpt = ((float)el->pktReceived)/totalTime;
	  el->averageSentPktThpt = ((float)el->pktSent)/totalTime;

	  if(topSentIdx == -1) {
	    topSentIdx = idx;
	  } else {
	    if(el->actualSentThpt > device[deviceToUpdate].hash_hostTraffic[topSentIdx]->actualSentThpt) {
	      secondSentIdx = topSentIdx;
	      topSentIdx = idx;
	    } else {
	      if(secondSentIdx == -1)
		secondSentIdx = idx;
	      else {
		if(el->actualSentThpt > device[deviceToUpdate].hash_hostTraffic[secondSentIdx]->actualSentThpt) {
		  thirdSentIdx = secondSentIdx;
		  secondSentIdx = idx;
		} else {
		  if(thirdSentIdx == -1)
		    thirdSentIdx = idx;
		  else {
		    if(el->actualSentThpt > device[deviceToUpdate].hash_hostTraffic[thirdSentIdx]->actualSentThpt) {
		      thirdSentIdx = idx;
		    }
		  }
		}
	      }
	    }
	  }

	  if(topRcvdIdx == -1) {
	    topRcvdIdx = idx;
	  } else {
	    if(el->actualRcvdThpt > device[deviceToUpdate].hash_hostTraffic[topRcvdIdx]->actualRcvdThpt) {
	      secondRcvdIdx = topRcvdIdx;
	      topRcvdIdx = idx;
	    } else {
	      if(secondRcvdIdx == -1)
		secondRcvdIdx = idx;
	      else {
		if(el->actualRcvdThpt > device[deviceToUpdate].hash_hostTraffic[secondRcvdIdx]->actualRcvdThpt) {
		  thirdRcvdIdx = secondRcvdIdx;
		  secondRcvdIdx = idx;
		} else {
		  if(thirdRcvdIdx == -1)
		    thirdRcvdIdx = idx;
		  else {
		    if(el->actualRcvdThpt > device[deviceToUpdate].hash_hostTraffic[thirdRcvdIdx]->actualRcvdThpt) {
		      thirdRcvdIdx = idx;
		    }
		  }
		}
	      }
	    }
	  }

	  if(updateHourThpt) {
	    el->lastHourRcvdThpt = ((float)(el->bytesReceived-el->lastHourBytesReceived)/timeHourDiff)/* *8 */;
	    el->lastHourSentThpt = ((float)(el->bytesSent-el->lastHourBytesSent)/timeHourDiff)/* *8 */;
	    el->lastHourBytesReceived = el->bytesReceived;
	    el->lastHourBytesSent = el->bytesSent;

	    if(topHourSentIdx == -1) {
	      topHourSentIdx = idx;
	    } else {
	      if(el->lastHourSentThpt > device[deviceToUpdate].hash_hostTraffic[topHourSentIdx]->lastHourSentThpt) {
		secondHourSentIdx = topHourSentIdx;
		topHourSentIdx = idx;
	      } else {
		if(secondHourSentIdx == -1)
		  secondHourSentIdx = idx;
		else {
		  if(el->lastHourSentThpt > device[deviceToUpdate].hash_hostTraffic[secondHourSentIdx]->lastHourSentThpt) {
		    thirdHourSentIdx = secondHourSentIdx;
		    secondHourSentIdx = idx;
		  } else {
		    if(thirdHourSentIdx == -1)
		      thirdHourSentIdx = idx;
		    else {
		      if(el->lastHourSentThpt > device[deviceToUpdate].hash_hostTraffic[thirdHourSentIdx]->lastHourSentThpt) {
			thirdHourSentIdx = idx;
		      }
		    }
		  }
		}
	      }
	    }

	    if(topHourRcvdIdx == -1) {
	      topHourRcvdIdx = idx;
	    } else {
	      if(el->lastHourRcvdThpt > device[deviceToUpdate].hash_hostTraffic[topHourRcvdIdx]->lastHourRcvdThpt) {
		secondHourRcvdIdx = topHourRcvdIdx;
		topHourRcvdIdx = idx;
	      } else {
		if(secondHourRcvdIdx == -1)
		  secondHourRcvdIdx = idx;
		else {
		  if(el->lastHourRcvdThpt > device[deviceToUpdate].hash_hostTraffic[secondHourRcvdIdx]->lastHourRcvdThpt) {
		    thirdHourRcvdIdx = secondHourRcvdIdx;
		    secondHourRcvdIdx = idx;
		  } else {
		    if(thirdHourRcvdIdx == -1)
		      thirdHourRcvdIdx = idx;
		    else {
		      if(el->lastHourRcvdThpt > device[deviceToUpdate].hash_hostTraffic[thirdHourRcvdIdx]->lastHourRcvdThpt) {
			thirdHourRcvdIdx = idx;
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }

    /* ******************************** */

    device[deviceToUpdate].throughput = device[deviceToUpdate].ethernetBytes-device[deviceToUpdate].throughput;
    device[deviceToUpdate].packetThroughput = device[deviceToUpdate].ethernetPkts-device[deviceToUpdate].lastNumEthernetPkts;
    device[deviceToUpdate].lastNumEthernetPkts = device[deviceToUpdate].ethernetPkts;
    device[deviceToUpdate].actualThpt = ((float)device[deviceToUpdate].throughput/(float)timeDiff)/* *8 */;
    device[deviceToUpdate].actualPktsThpt = (float)device[deviceToUpdate].packetThroughput/(float)timeDiff;

    if(device[deviceToUpdate].actualThpt > device[deviceToUpdate].peakThroughput)
      device[deviceToUpdate].peakThroughput = device[deviceToUpdate].actualThpt;

    if(device[deviceToUpdate].actualPktsThpt > device[deviceToUpdate].peakPacketThroughput)
      device[deviceToUpdate].peakPacketThroughput = device[deviceToUpdate].actualPktsThpt;

    device[deviceToUpdate].throughput = device[deviceToUpdate].ethernetBytes;
    device[deviceToUpdate].packetThroughput = device[deviceToUpdate].ethernetPkts;

    if(updateMinThpt) {
      device[deviceToUpdate].lastMinEthernetBytes = device[deviceToUpdate].ethernetBytes - device[deviceToUpdate].lastMinEthernetBytes;
      device[deviceToUpdate].lastMinThpt = ((float)(device[deviceToUpdate].lastMinEthernetBytes)/(float)timeDiff)/* *8 */;
      device[deviceToUpdate].lastMinEthernetBytes = device[deviceToUpdate].ethernetBytes;
      /* ******************* */
      device[deviceToUpdate].lastMinEthernetPkts = device[deviceToUpdate].ethernetPkts - device[deviceToUpdate].lastMinEthernetPkts;
      device[deviceToUpdate].lastMinPktsThpt = (float)device[deviceToUpdate].lastMinEthernetPkts/(float)timeDiff;
      device[deviceToUpdate].lastMinEthernetPkts = device[deviceToUpdate].ethernetPkts;
      device[deviceToUpdate].lastMinThptUpdate = actTime;
    }

    if((timeDiff = actTime-device[deviceToUpdate].lastFiveMinsThptUpdate) > 300 /* 5 minutes */) {
      device[deviceToUpdate].lastFiveMinsEthernetBytes = device[deviceToUpdate].ethernetBytes - device[deviceToUpdate].lastFiveMinsEthernetBytes;
      device[deviceToUpdate].lastFiveMinsThptUpdate = timeDiff;
      device[deviceToUpdate].lastFiveMinsThpt = ((float)device[deviceToUpdate].lastFiveMinsEthernetBytes/(float)device[deviceToUpdate].lastFiveMinsThptUpdate)/* *8 */;
      device[deviceToUpdate].lastFiveMinsEthernetBytes = device[deviceToUpdate].ethernetBytes;
      /* ******************* */
      device[deviceToUpdate].lastFiveMinsEthernetPkts = device[deviceToUpdate].ethernetPkts - device[deviceToUpdate].lastFiveMinsEthernetPkts;
      device[deviceToUpdate].lastFiveMinsPktsThpt = (float)device[deviceToUpdate].lastFiveMinsEthernetPkts/(float)device[deviceToUpdate].lastFiveMinsThptUpdate;
      device[deviceToUpdate].lastFiveMinsEthernetPkts = device[deviceToUpdate].ethernetPkts;
      device[deviceToUpdate].lastFiveMinsThptUpdate = actTime;
    }

    if((updateMinThpt || updateHourThpt) 
       && ((topSentIdx    != -1) 
	   || (topHourSentIdx != -1)
	   || (topRcvdIdx     != -1)
	   || (topHourRcvdIdx != -1)))
      updateThptStats(deviceToUpdate,
		      topSentIdx, secondSentIdx, thirdSentIdx,
		      topHourSentIdx, secondHourSentIdx, thirdHourSentIdx,
		      topRcvdIdx, secondRcvdIdx, thirdRcvdIdx,
		      topHourRcvdIdx, secondHourRcvdIdx, thirdHourRcvdIdx);

    device[deviceToUpdate].lastThptUpdate = actTime;
  }
}

/* ******************************* */

void updateThpt() {
  int i;

  if(mergeInterfaces)
    updateDeviceThpt(0);
  else {
    for(i=0; i<numDevices; i++)
      updateDeviceThpt(i);  
  }
}


/* ******************************* */

void updateTrafficMatrix(HostTraffic *srcHost,
			 HostTraffic *dstHost,
			 TrafficCounter length) {
  if(subnetLocalHost(srcHost) && subnetLocalHost(dstHost)) {
    unsigned long a = (unsigned long)(srcHost->hostIpAddress.s_addr) % 256 /* C-class */;
    unsigned long b = (unsigned long)(dstHost->hostIpAddress.s_addr) % 256 /* C-class */;

    ipTrafficMatrixHosts[a] = srcHost, ipTrafficMatrixHosts[b] = dstHost;
    ipTrafficMatrix[a][b].bytesSent += length,
      ipTrafficMatrix[b][a].bytesReceived += length;
  }
}

/* *********************************** */

void updateDbHostsTraffic(int deviceToUpdate) {
  u_int i;
  HostTraffic *el;

#ifdef DEBUG
  traceEvent(TRACE_INFO, "updateDbHostsTraffic()\n");
#endif

  for(i=0; i<device[actualDeviceId].actualHashSize; i++) {
    el = device[deviceToUpdate].hash_hostTraffic[i]; /* (**) */

    if((el != NULL)
       && (!broadcastHost(el))
       && (el->nextDBupdate < actTime)) {

      el->instanceInUse++;

      if(el->nextDBupdate == 0)
	notifyHostCreation(el);
      else if(el->nextDBupdate < actTime) {
	updateHostTraffic(el);
	if(el->osName == NULL) {
	  updateOSName(el);
	}
      }

      el->nextDBupdate = actTime + DB_TIMEOUT_REFRESH_TIME;
      el->instanceInUse--;
    }
  }
}
