Characterize the *Connext DDS* performance of an environment using RTI Perftest
===============================================================================

This article is meant to be a quick guide about the initial steps we recommend to follow to profile and
characterize the performance between 2 machines in a given environment. This means understanding the maximum
throughput that **RTI Connext DDS** can maintain in a 1 to 1 communication, as well as the average latency we
can expect when sending packets.

For this demonstration we will use a couple Raspberry Pi boards, connected to a switch. See below the
information about the environment

   | Target machines: **Raspberry Pi 3B+**
   |                  OS:
   |                  NIC: 100Mbps
   | Switch: 1Gbps switch
   | //TODO add info about the machines to use ``

Prepare the tools
~~~~~~~~~~~~~~~~~

In order to perform this test we will only need **RTI Perftest 3.0** (Perftest). We will compile it against **RTI Connext DDS
Professional 6.0.0** and **RTI Connext DDS Micro 3.0.0**.

Get Perftest
^^^^^^^^^^^^

-  You can clone it from the official *Github* repository:

   Go to the `release_page <https://github.com/rticommunity/rtiperftest/releases>`_ for **Perftest** and
   check what is the latest release, then clone that release using `git`. At this point the latest release is 3.0:

     | ``git clone -b release/3.0 https://github.com/rticommunity/rtiperftest.git``

   This command will download the *Github* repository in a folder named
   ``rtiperftest`` and move to the ``release/3.0`` tag.
   If you don't include the ``-b release/3.0``, you will clone the ``master`` branch
   of the product.

-  You can download a `zip` file containing the **Perftest** source files from
   the **Github** page:
   `github.com/rticommunity/rtiperftest <https://github.com/rticommunity/rtiperftest>`__.
   Once the zip file is downloaded you will need to extract its content,
   this will create a folder named ``rtiperftest``.

-  You can download a `zip/tar.gz` file containing the **Perftest**executable statically
   compiled for some of the most common platforms from the **Github** release page:
   `https://github.com/rticommunity/rtiperftest/releases <https://github.com/rticommunity/rtiperftest/releases>`__
   in the "Binaries" section. Once the zip file is downloaded you will need to extract its content, this will
   create a folder with the binaries for your architecture.

All this information is also covered in the `download <https://github.com/rticommunity/rtiperftest/blob/3.0/srcDoc/download.rst>`__
section of the **Perftest** documentation.

Compile against RTI Connext DDS 6.0.0
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you already got the compiled binaries, you can skip this step. Else, you will need to compile the
binaries by yourself. This process is covered in the `compilation <https://github.com/rticommunity/rtiperftest/blob/3.0/srcDoc/compilation.rst>`__
section of the **Perftest** documentation, so we will just assume all the steps are followed.


Tests
~~~~~