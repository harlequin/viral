/*** REMOTE PROCEDURE CALLS VIA JSON-RPC *************************************************/

var RPC = (new function($)
{
	'use strict';
	
	// Properties
	this.rpcUrl = "/jsonrpc";
	//this.defaultFailureCallback;
	this.connectErrorMessage = 'Cannot establish connection';

	
	this.call = function(method, params, completed_callback, failure_callback)
	{
		var _this = this;
		
		var request = JSON.stringify({nocache: new Date().getTime(), id: 1, method: method, params: params});
		var xhr = new XMLHttpRequest();
		xhr.open('post', this.rpcUrl);

		xhr.onreadystatechange = function()	{
			if (xhr.readyState === 4) {
				var res = 'Unknown error';
				var result;
				if (xhr.status === 200) {
					if (xhr.responseText != '')	{
						try	{							
							result = JSON.parse(xhr.responseText);
						} catch (e)	{						
							res = e;								
						}
						
						if (result)	{			
							if (result.error == null) {								
								res = result.result;
								if(completed_callback) {
									completed_callback(res, result.version, result.hash);
								}								
								
								return;
							} else {
								res = result.error.message + '<br><br>Request: ' + request;
							}
						}
					} else {
						res = 'No response received.';						
					}
				} else if (xhr.status === 0) {
					res = _this.connectErrorMessage;												
				} else {
					res = 'Invalid Status: ' + xhr.status;					
				}
				
				//When we are reaching here a fault occured				
				if (failure_callback) {
					failure_callback(res, result);
				} else {
					//_this.defaultFailureCallback(res, result);
				}
			}
		};
		xhr.send(request);
	}
}(jQuery));