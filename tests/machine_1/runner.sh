#!/bin/sh
set -e

cat <<'EOF'
    VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
     VVVAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAVVV
      VVVAAAAAAAAAAAAAAAAAAAAAAAAAAAAAVVV
       VVVAAA                     AAAVVV
        VVVAAA                   AAAVVV
         VVVAAA                 AAAVVV
     /\      //^^^^\\    ||      ||     /\
    //\\    ||      ||   ||      ||    //\\
   //  \\   ||      ||   ||      ||   //  \\
  //    \\  ||      ||   ||      ||  //    \\
 //      \\  \\____//____ \\____//  //      \\
               VVVAAA     AAAVVV
                VVVAAA   AAAVVV
                 VVVAAA AAAVVV
                  VVVAAAAAVVV
                   VVVAAAVVV
                    VVVAVVV
                     VVVVV
                      VVV
                       V
EOF

sleep .1
iface=$(ifconfig | grep epair | head -n1 | cut -d: -f1)
ifconfig
ifconfig $iface inet $IP/24

export PATH=$PATH:/usr/local/bin
export LD_LIBRARY_PATH=/usr/local/lib

export UMBER_LINEBUF=true
export UMBER_LVL=*=i,aqua.gvd.init=v,test=v,aqua.kos*=v

gvd -i $iface &
sleep 2 # Wait for connection to other node.

machine-test
