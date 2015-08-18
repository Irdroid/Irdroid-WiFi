#!/bin/ash

echo "Content-type: text/html"
echo ""

echo '<html>'
echo '<head>'
echo '<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">'
echo '<title>Environment Variables</title>'
echo '<link rel="stylesheet" type="text/css" href="http://192.168.2.1/styles.css" />'
echo '</head>'
echo '<body>'


# Save the old internal field separator.
  OIFS="$IFS"
  
  # Set the field separator to & and parse the QUERY_STRING at the ampersand.
    IFS="${IFS}&"
      set $QUERY_STRING
        Args="$*"
          IFS="$OIFS"
          
          # Next parse the individual "name=value" tokens.
          
            ARGX=""
              ARGY=""
                ARGZ=""
                
                  for i in $Args ;do
                  
                  #       Set the field separator to =
                          IFS="${OIFS}="
                                  set $i
                                          IFS="${OIFS}"
                                          
                                                  case $1 in
                                                                  # Don't allow "/" changed to " ". Prevent hacker problems.
                                                                                  namex) ARGX="`echo $2 | sed 's|[\]||g' | sed 's|%20| |g'`"
                                                                                                         ;;
                                                                                                                         # Filter for "/" not applied here
                                                                                                                                         namey) ARGY="`echo $2 | sed 's|%20| |g'`"
                                                                                                                                                                ;;
                                                                                                                                                                                namez) ARGZ="${2/\// /}"
                                                                                                                                                                                                       ;;
                                                                                                                                                                                                                       *)     echo "<hr>Warning:"\
                                                                                                                                                                                                                                                   "<br>Unrecognized variable \'$1\' passed by FORM in QUERY_STRING.<hr>"
                                                                                                                                                                                                                                                                          ;;
                                                                                                                                                                                                                                                                          
                                                                                                                                                                                                                                                                                  esac
                                                                                                                                                                                                                                                                                    done
                                                                                                                                                                                                                                                                                    echo '<div id="container">'
                                                                                                                                                                                                                                                                                    echo '<div id="listingcontainer">'
                                                                                                                                                                                                                                                                                    echo '<div id="listingheader">'
                                                                                                                                                                                                                                                                                    echo '<div id="listing">'
                                                                                                                                                                                                                                                                                      echo '<H2>Downloading remote file:</H2>' 
                                                                                                                                                                                                                                                                                        echo '<br>'
                                                                                                                                                                                                                                                                                        
                                                                                                                                                                                                                                                                                         cp /etc/lircd.conf /etc/lircdtmp.conf
                                                                                                                                                                                                                                                                                         rm /etc/lircd.conf
                                                                                                                                                                                                                                                                                         /usr/bin/wget $ARGX
                                                                                                                                                                                                                                                                                        echo '<br>'
                                                                                                                                                                                                                                                                                        echo '<br>'
                                                                                                                                                                                                                                                                                         cat /www/cgi-bin/$ARGY  /etc/lircdtmp.conf > /etc/lircd.conf
                                                                                                                                                                                                                                                                                         rm /www/cgi-bin/$ARGY
                                                                                                                                                                                                                                                                                         rm /etc/lircdtmp.conf
                                                                                                                                                                                                                                                                                         echo '<br>'
                                                                                                                                                                                                                                                                                         echo 'Done!'
                                                                                                                                                                                                                                                                                        killall -sigkill lircd
                                                                                                                                                                                                                                                                                         sleep 3
                                                                                                                                                                                                                                                                                     /usr/sbin/lircd --device=/dev/ttyACM0 --listen=192.168.2.1:8765  
                                                                                                                                                                                                                                                                                            echo '<br>'
                                                                                                                                                                                                                                                                                           
                                                                                                                                                                                                                                                                                                echo '<br>'
                                                                                                                                                                                                                                                                                                  echo $ARGZ
                                                                                                                                                                                                                                                                                                 echo '</div>'
                                                                                                                                                                                                                                                                                                 echo '</div>'
                                                                                                                                                                                                                                                                                                 echo '</div>'
                                                                                                                                                                                                                                                                                                 echo '</div>' 
                                                                                                                                                                                                                                                                                                  echo '</body>'
                                                                                                                                                                                                                                                                                                  echo '</html>'
                                                                                                                                                                                                                                                                                                  
                                                                                                                                                                                                                                                                                                  exit 0
