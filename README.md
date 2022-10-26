# Traffic Aware Fast Reroute Mechanism (TA-FRR) Exploiting Disjoint Sub-Paths for Named Data Networks 

 *********************************************************************************
This work is licensed under CC BY-NC-SA 4.0
(https://creativecommons.org/licenses/by-nc-sa/4.0/).

Copyright (c) 2022 zhouby-zjl @ github

This file is a part of "Traffic Aware Fast Reroute Mechanism (TA-FRR)"
(https://github.com/zhouby-zjl/tafrr/).

Written by zhouby-zjl @ github

This software is protected by the patents as well as the software copyrights.
**********************************************************************************
  
## What is the TA-FRR? 
The fast reroute (FRR) is required by computer networks for enhancing resilience in dealing with the norm of link fault. TA-FRR is a novel  traffic aware fast re-route (TA-FRR) mechanism based on the popularized named data network (NDN), with exploitation of a new resilient sub-graph (RSG) routes involving multiple disjoint sub-paths (DSPs). TA-FRR proactively constructs an RSG with a scalable computation complexity to maximize the detouring resiliency and also to avoid too higher reroute stretch. With the RSG, TA-FRR efficiently compute the shortest detouring path for the disconnected components identified according to packet arrival timeouts in an active manner. TA-FRR can efficiently recover serious link failures in the PP, with a scalable RSG construction, and also outperforms the existing arborescences and disjoint paths based approaches with a significantly higher resiliency and a similar stretch to the arborescences based one. 

## What is the status of the TA-FRR?
The DRTP is currently in the prototype stage which is comprehensively tested under the Internet topologies synthesized by the BRITE tool. We are looking forward to implement into a real router in the future.

## How do I run the source code?
1. Download TA-FRR source code from github and the recent version of ndnSIM from https://ndnsim.net/current/. 
2. Simply copy all files of the TA-FRR source code under the ndnSIM-frr directory to the original ndnSIM directory in an override manner. 
3. You need download BRITE topology generator from https://www.cs.bu.edu/brite/index.html, and synthesizes a router-level topology using BRITE
3. You need to edit the link-failover-brite.ini file under the ndnSIM/ns-3 folder with the correct synthesized BRITE topology path, the output log path and some simulation parameters. 
4. cd ndnSIM/ns-3 && ./waf --run scratch/link-failover-icc --command-template="%s link-failover-brite.ini 1 2". 
5. Afterwards, you can find the simulation results under the SIM_LOG_DIR directory defined in the link-failover-brite.ini file.

**********************************************************************************
We are looking forward to new project opportunity in making the TA-FRR growing up. 

