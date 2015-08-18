
	var buffer = 0;
	var x_walker = 0, last_x=0, last_y=0;
	var socket;
	var host = "ws://localhost:5331/write_anything_here.php";
	
$(document).ready(function() {
	

	function connect(){

			try{
				
				socket = new WebSocket(host);
				var in_string;
				message('<p class="event">Socket Status: '+socket.readyState);
				
				socket.onopen = function(){
					message('<p class="event">Socket Status: '+socket.readyState+' (open)');	
				}
				
				socket.onmessage = function(msg){	
				
					var in_string = new String(msg.data);
					var eol_pos = in_string.indexOf("\n",0);
					
					if( eol_pos != -1 ){
						// we have eol
		
						var start_of_string = in_string.substring( 0, eol_pos-1 );
						var rest_of_string = in_string.substring( eol_pos+1 );
						buffer = buffer + start_of_string;
							
						// command reply or values?
						
						if( buffer.indexOf("Command",0) != -1 ){
						
							message('<p class="message">Response from serial : ' + buffer );
							
						}else{

							// parse value
							var val_pos = buffer.split(" ");
							 // message('<p class="message">Value : ' + val_pos[0] + " and " + val_pos[1] );
							
							plotCanvas(x_walker, parseInt( val_pos[1] ) + parseInt( val_pos[0] ));					
							if( x_walker++ > 640 ) x_walker = 0;
							
						}
						buffer = rest_of_string;
					}else{
						// no eol in this message, we wait for it
						buffer = buffer + in_string;
					}
			
				}
				
				socket.onclose = function(){
					message('<p class="event">Socket Status: '+socket.readyState+' (Closed)');
					socket.close();
				}			
					
			} catch(exception){
				message('<p>Error'+exception);
			}
			
		}
			
		function send(){
		
			var text = $('#text').val();

			if( text == "" ) return;	
				
			try{
				socket.send(text);
				message('<p class="event">Sent: '+text);
				$('#text').val("");
			} catch(exception){
				message('<p class="warning">Connection problem.</p>');
			}
			
		}
	
		function plotCanvas(x,y){
			var context = $('#dataCanvas')[0].getContext("2d");
			if( y > last_y ) context.strokeStyle = "red";
			else context.strokeStyle = "green";
			context.lineJoin = "round";
			context.lineWidth = 2;
			context.beginPath();
			context.moveTo(last_x, last_y);
			context.lineTo(x, y);
			context.closePath();
			context.stroke();
			last_x = x;
			last_y = y;
		}
			
		function message(msg){
			$('#dataLog').append( msg + "<br />" );
			$("#dataLog").animate({ scrollTop: $("#dataLog").attr("scrollHeight") }, 250);
		}
			
		$('#text').keypress(function(event) {
			if (event.keyCode == '13') {
				 send();
			}
		});	
			
		$('#send').click(function(){
			send();
		});
			
		$('#disconnect').click(function(){
			message( "Disconnecting " + "<br />" );
			socket.close();
		});	
	
		$('#connect').click(function(){
			message( "Connecting " + "<br />" );
			connect();
		});

	
});