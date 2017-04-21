var search = (new function($) {
	'use strict';
	var timeout;
	
	this.init = function() {
			
		$("#search-table").dynatable({
			 features: {
				paginate: false,
				sort: false,
				//pushState: false,
				search: false,
				recordCount: true,
				perPageSelect: false
			},			
			table: {
			    bodyRowSelector: 'li'
			},
			writers: {
				_rowWriter: item_writer
			},
		});
	};
	
	function item_writer(idx, record, columns, cell) {
		var li, img, btn_img, ev;
		console.log(record);
		var cssClass = "col-sm-2 col-xs-10";
		//if (idx % 4 === 0) { cssClass += ' first'; }
		
		
		
		//li = '<li class="' + cssClass + '"><div class="panel panel-danger"><div class="panel-heading"> '+ record.name +'</div><div class="panel-body"><a data-toggle="tab" href="#series-details-tab"><img src="'+img+'" class="img-rounded"></a></div><div class="panel-footer"><button class="btn btn-success" type="submit" id="series-add" data-event="'+ev+'" data-key="'+record.id+'" data-value="'+record.name+'"><i class="glyphicon '+btn_img+'"></i></button></div></div></li>';
		//li = '<li class="' + cssClass + '"><div class="panel panel-danger"><div class="panel-heading"> '+ record.name +'</div><div class="panel-body"><a id="show-details" data-id="' + record.id + '"><img src="'+img+'" class="img-rounded"></a></div><div class="panel-footer"><button class="btn btn-success" type="submit" id="series-add" data-event="'+ev+'" data-key="'+record.id+'" data-value="'+record.name+'"><i class="glyphicon '+btn_img+'"></i></button></div></div></li>';
		
		
		li = `
		<div class="panel panel-info">
			<div class="panel-heading clearfix">
				<span class="panel-title pull-left" style="max-width: 75%;white-space: nowrap;white-space: nowrap;padding-top: 7.5px; text-overflow: ellipsis;overflow:hidden !important;">` + record.name + ` </span>
				
				<div class="btn-group pull-right" role="group" aria-label="...">
					
					<button type="button" class="btn btn-success btn-sm" 
						data-server="`+ record.naddr +`" 
						data-msg="/msg `+record.uname+` xdcc send #`+record.n+`" 
						data-channel="` + record.cname + `"
					>
					
					<span class="glyphicon glyphicon glyphicon-download-alt" aria-hidden="true"></span></button>																		
				</div>
			</div>
			<div class="panel-body">
				<div class="row" >
					<div class="col-xs-6">
						<small>
						` + record.naddr + `-` + record.uname +` 			
						</small>
					</div>
					<div class="col-xs-6">
						<small>
						` + record.cname + ` <br /> ` + record.szf + `
						</small>
					</div>
				</div>								
			</div>
		</div>
		`;
		
		
	
		return li;
	}
	
	$("#search-table").on('click', 'button', function(){		
		RPC.call("download_add", [$(this).attr('data-msg'), "", $(this).attr('data-server'), $(this).attr('data-channel') ]);
	});

	this.search = function( value ) {
		console.log("search ixirc ...");
		RPC.call("search_ixirc", [value], function(items){			//
			var itm = items[0].results;
			
			var dynatable = $('#search-table').data('dynatable');
			//console.log(itm);
			dynatable.records.updateFromJson({records: itm});
			dynatable.records.init();
			dynatable.process();
		});
	};
	
	
}(jQuery));