/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.harness;

import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;
import com.rti.perftest.harness.PerfTest;

public class PerftestTimerTask extends TimerTask {

  private PerfTest perftest;
  private Timer timer;
  private boolean _isScan;
  
  public PerftestTimerTask(PerfTest _perftest) {
      // this way we can modify testCompleted in perftest
      perftest = _perftest;
      // running timer task as daemon thread (true)
      timer = new Timer(true);
      _isScan = false;
  }
  
  @Override
  public void run() {
      if (!_isScan) {
          perftest.finishTest();
      } else {
          perftest.finishTestScan();
      }
  }

  public void setTimeout(long executionTime, boolean isScan) {
      timer.schedule(this, executionTime * 1000);
      _isScan = isScan;
      if (!_isScan) {
          System.err.println("Setting timeout to "
                + executionTime + " seconds.");
      }
  }
  
  public void cancelTimer() {
      timer.cancel();
  }

}
// ===========================================================================
