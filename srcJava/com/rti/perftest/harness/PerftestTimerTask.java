/*
 * Copyright 2016
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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