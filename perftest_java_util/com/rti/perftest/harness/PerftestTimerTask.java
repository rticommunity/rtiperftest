/* $Id: PerftestTimerTask.java,v 1.1 2014/08/27 11:35:57 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
5.1.0,27aug14,jmc PERFTEST-63 Created.
=========================================================================== */

package com.rti.perftest.harness;

import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;
import com.rti.perftest.harness.PerfTest;

public class PerftestTimerTask extends TimerTask {

  private PerfTest perftest;
  private Timer timer;
  
  public PerftestTimerTask(PerfTest _perftest) {
      // this way we can modify testCompleted in perftest
      perftest = _perftest;
      // running timer task as daemon thread (true)
      timer = new Timer(true);
  }
  
  @Override
  public void run() {
      perftest.finishTest();
  }

  public void setTimeout(long executionTime) {
      timer.schedule(this, executionTime * 1000);
      System.err.println("Setting timeout to " + executionTime + " seconds.");
  }
  
  public void cancelTimer() {
      timer.cancel();
  }

}
// ===========================================================================
// End of $Id: PerftestTimerTask.java,v 1.1 2014/08/27 11:35:57 juanjo Exp $