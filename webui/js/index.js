/*** CONFIRMATION DIALOG *****************************************************/

var ConfirmDialog = (new function($)
{
	'use strict';

	// Controls
	var $ConfirmDialog;
	
	// State
	var actionCallback;
	
	this.init = function()
	{
		$ConfirmDialog = $('#ConfirmDialog');
		$ConfirmDialog.on('hidden', hidden);
		$('#ConfirmDialog_OK').click(click);
	}

	this.showModal = function(id, callback, loaded_callback)	{
				
	
		$('#ConfirmDialog_Title').html($('#' + id + '-title').html());
		$('#ConfirmDialog_Text').html($('#' + id + '-text').html());
		$('#ConfirmDialog_OK').html($('#' + id + '-ok').html());
		actionCallback = callback;
				
		$ConfirmDialog.modal({backdrop: 'static'});
				
		// avoid showing multiple backdrops when the modal is shown from other modal
		var backdrops = $('.modal-backdrop');
		if (backdrops.length > 1)
		{
			backdrops.last().remove();
		}
		
		
		if(loaded_callback)
			loaded_callback();
		
	}

	function hidden()
	{
		// confirm dialog copies data from other nodes
		// the copied DOM nodes must be destroyed
		$('#ConfirmDialog_Title').empty();
		$('#ConfirmDialog_Text').empty();
		$('#ConfirmDialog_OK').empty();
	}

	function click(event)
	{
		event.preventDefault(); // avoid scrolling
		if(actionCallback)
			actionCallback();
		$ConfirmDialog.modal('hide');
	}
}(jQuery));


var Frontend = (new function($) {
	'use strict';

	this.init = function() {
		
		ConfirmDialog.init();		
				
		$("#shutdown").click(function() {
			ConfirmDialog.showModal('shutdown-confirm-dialog', function(){
				console.log("Shutdown");
				RPC.call("shutdown");
			});
		});		
		
		$('#manual_post_process').click(function() {			
			ConfirmDialog.showModal('manual-post-process-dialog', function(){							
				RPC.call("post_process_manual", [$('#manual-post-process-path').val()]);
			}, function(){
				$('#manual-post-process-path').val("/Data/Public/downloads");	
			});
		});
		
		/* refresh downloads table every 2 seconds */
		setInterval( function () {			
			downloads.update();
		}, 2000);
		
	}
	
	
} (jQuery));




$(document).ready(function() {	
	Frontend.init();
	downloads.init();	
	logs.init();
	series.init();
	search.init();
	
	RPC.call("version", [], function(items){
		console.log(items);
		$('#version-label').text(items[0].version);
		$('#date-label').text(items[0].date);		
	});
			
	$(document).on('click', 'form[role="search"] button[type="submit"]', function(event){
		var value = $('#search-text').val();		
		$('a[href="#search-tab"]').tab('show');
		if ( value != '' ) {
			search.search(value);				
		}		
		
		$('#search-text').val("");
		event.preventDefault();	
	}); 
	
});
