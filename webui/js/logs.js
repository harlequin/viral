var logs = (new function($) {
	'use strict';
	
	this.init = function() {
		$("#logs-table").dynatable({
			 features: {
				paginate: false,
				sort: false,				
				search: false,
				recordCount: true,
				perPageSelect: false
			},
			writers: {
				
								
				_attributeWriter: function(record) {
					
					
					var level = "Off";
					
					switch (record.level) {
					case 1:
						level = "Fatal";
						break;
					case 2:
						level = "Error";
						break;
					case 3:
						level = "Warning";
						break;
					case 4:
						level = "Info";
						break;
					case 5:
						level = "Debug";
						break;
					default:
						break;
					}								
					
					
					var item = {
						level: '<span class="label label-' + level + '">' + level + '</span>',
						data : record.data,
					};
					return item[this.id];
				}
			},
			table: {
			}
		});
	};
	
	this.update = function() {
		
		RPC.call("log", [], function(items){
			console.log(items);
			var dynatable = $('#logs-table').data('dynatable');
			dynatable.records.updateFromJson({records: items});
			dynatable.records.init();
			dynatable.process();
		});
	};
	
}(jQuery));