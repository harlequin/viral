var downloads = (new function($)
{
	'use strict';

	var status_table = new Array();
	var color_status_table = new Array();

	function readablizeBytes (bytes) {
		if ( bytes == 0 )
			return "0 bytes";
	    var s = ['bytes', 'kB', 'MB', 'GB', 'TB', 'PB'];
	    var e = Math.floor(Math.log(bytes) / Math.log(1024));
	    return (bytes / Math.pow(1024, e)).toFixed(2) + " " + s[e];
	}

	this.init = function() {

		status_table[0] = "Ready";
		status_table[1] = "Requested";
		status_table[2] = "Queued";
		status_table[3] = "Downloading";
		status_table[4] = "Downloaded";
		status_table[5] = "Postprocess";
		status_table[6] = "Completed";
		status_table[7] = "History";
		status_table[8] = "Paused";
		status_table[9] = "Error";
		
		color_status_table[0] = "default";
		color_status_table[1] = "info";
		color_status_table[2] = "warning";
		color_status_table[3] = "primary";
		color_status_table[4] = "success";
		color_status_table[5] = "default";
		color_status_table[6] = "default";
		color_status_table[7] = "default";
		color_status_table[8] = "default";
		color_status_table[9] = "danger";



		$('#download_add').click(this.add);
		$("#downloads-table").dynatable({
				 features: {
					paginate: false,
					sort: false,
					//pushState: false,
					search: false,
					recordCount: true,
					perPageSelect: false
				},
				
				writers: {
					_attributeWriter: function(record) {
						console.log(record);
						var progress = Math.floor(record.current_size / record.total_size * 100);
						var progress_bar = '<div class="progress"><div class="progress-bar" role="progressbar" aria-valuenow="' + progress + '" aria-valuemin="0" aria-valuemax="100" style="width: ' + progress + '%;">' + record.name +'</div></div>';
						var item = {
							action : '<button id="download_delete" type="button" class="btn btn-danger" data-id="' + record.id + '">Delete</button>',
							bot : record.bot,
							channel : record.channel,
							current : readablizeBytes(record.current_size),
							id  : record.id,
							name : progress_bar /*record.name*/,
							package : record.package,
							status : '<span class="label label-' + color_status_table[record.status] + '">' + status_table[record.status] + '</span>',
							server : record.server,
							total : readablizeBytes(record.total_size),
							progress : progress + ' %',
							speed : readablizeBytes(record.speed) + '/sec',
							error : record.error,
						};
						//;
						return item[this.id];
					}
				},
				table: {
				    //defaultColumnIdStyle: 'underscore'
				}
		});
		
		this.update();
	};


	$("#downloads-table").on('click', '#download_delete', function(){
		var id = $(this).attr('data-id');			
		ConfirmDialog.showModal('delete-download-confirm-dialog', function(){
			RPC.call('download_delete', [id * 1], function(items){
				downloads.update();
			});				
		});		
	 });
	
	this.add = function() {
		ConfirmDialog.showModal('add-irc-download-dialog');
		var server_select;
		var server_items;
		var i;

		ConfirmDialog.showModal('add-irc-download-dialog', function(){
			/* add download to queue */
			console.log("Add download with parameter");
			console.log("Server " + server_items[server_select.val()].name);
			console.log($('#add-irc-download-dialog-channel').val());
			console.log($('#add-irc-download-dialog-message').val());

			RPC.call("download_add", [
			    $('#add-irc-download-dialog-message').val(),	/* Message */
			    server_items[server_select.val()].name, /* Server */
			    $('#add-irc-download-dialog-channel').val() /* Channel */
			]);

		}, function() {
			/* loading event */


			console.log("Load data into it");
			RPC.call('servers', [], function(items){
				console.log(items);
				server_items = items;
				server_select = $('#add-irc-download-dialog-server');
				$('#add-irc-download-dialog-server').find('option').remove().end()
					.change(function(){
						/* sever items will be selected */
						$('#add-irc-download-dialog-channel').val("");
						console.log("Server changed to ... " + $('#add-irc-download-dialog-server').val() + " - " + $('#add-irc-download-dialog-server').text());
						var id = $('#add-irc-download-dialog-server').val();
						$('#channels-dropdown').find('li').remove().end();
						for( i = 0; i< items[id].channels.length; i++) {
							$('#channels-dropdown').append($("<li><a href=\"#\" id=\"channel-select-"+i+"\">"+ items[id].channels[i] +"</a></li>"));
							jQuery("#channel-select-" + i).click(function(e){
								$('#add-irc-download-dialog-channel').val($(this).text());
								e.preventDefault();
							});
						};
					});

				for( i = 0; i< items.length; i++) {
					$('#add-irc-download-dialog-server').append($("<option />").val(i).text(items[i].name));
				}
				$('#add-irc-download-dialog-server').change();

			});
		});
	};

	this.update = function() {
		//console.log("Update data table");
		
		//RPC.call("log", []);
		
		RPC.call("downloads", ["test1", 20, 30], function(items){
			//console.log("----");
			//console.log(items);
			//$("#downloads-table").dynatable({ dataset: { records:items } });
			var dynatable = $('#downloads-table').data('dynatable');
			dynatable.records.updateFromJson({records: items});
			dynatable.records.init();
			//.dom.update();
			dynatable.process();

		});
	};
}(jQuery));
