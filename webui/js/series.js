var series = (new function($) {
	'use strict';
	var timeout;
	
	this.init = function() {
		
		
		$('#episode-table').dynatable({
			features: {
				paginate: false,
				sort: false,
				//pushState: false,
				search: false,
				recordCount: true,
				perPageSelect: false
			},
		});
		
		$("#series-table").dynatable({
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
		//console.log(record);
		var cssClass = "col-sm-2 col-xs-10";
		//if (idx % 4 === 0) { cssClass += ' first'; }
		
		img = record.poster;
		if ( img != "none.jpg") {
			if (record.poster.charAt(0) == "\\") {
				img = "http://image.tmdb.org/t/p/w154" + record.poster.substring(1);
			} else {
				img = "http://image.tmdb.org/t/p/w154" + record.poster;	
			}			
		}
		
		if ( record.status == "new" ) {
			btn_img = "glyphicon-plus";
			ev = "series_add";
		} else {
			btn_img = "glyphicon-search";
			ev = "search";
		}
		
		
		//li = '<li class="' + cssClass + '"><div class="panel panel-danger"><div class="panel-heading"> '+ record.name +'</div><div class="panel-body"><a data-toggle="tab" href="#series-details-tab"><img src="'+img+'" class="img-rounded"></a></div><div class="panel-footer"><button class="btn btn-success" type="submit" id="series-add" data-event="'+ev+'" data-key="'+record.id+'" data-value="'+record.name+'"><i class="glyphicon '+btn_img+'"></i></button></div></div></li>';
		li = '<li class="' + cssClass + '"><div class="panel panel-danger"><div class="panel-heading"> '+ record.name +'</div><div class="panel-body"><a id="show-details" data-id="' + record.id + '"><img src="'+img+'" class="img-rounded"></a></div><div class="panel-footer"><button class="btn btn-success" type="submit" id="series-add" data-event="'+ev+'" data-key="'+record.id+'" data-value="'+record.name+'"><i class="glyphicon '+btn_img+'"></i></button></div></div></li>';
		return li;
	}
	
	$('#series-table').on('click', 'a', function() {
		var id = $(this).attr('data-id');
		console.log(id);
		
		series.update_details(id);
		$('#viral-nav a[href="#series-details-tab"]').tab('show')
	});
	
	
	$("#series-table").on('click', 'button', function(){
		var series_id = $(this).attr('data-key');
		var series_event = $(this).attr('data-event');
		RPC.call(series_event, [series_id, 1, 1, 3]);		
	});
	
	$('#seasons_list').on('click', 'a', function(){
		var series_id = $(this).attr('data-key');
		var season = $(this).attr('data-season');
		console.log("click2 " + series_id + " - " + season);
		RPC.call("series_season", [ series_id, season ], function(items){
			//console.log(items);
			
			var dynatable = $('#episode-table').data('dynatable');
			dynatable.records.updateFromJson({records: items});
			dynatable.records.init();
			dynatable.process();
			
		});
	});
	
	this.search = function( value ) {
		RPC.call("search_themoviedb", [value], function(items){
			//console.log(items);
			var dynatable = $('#series-table').data('dynatable');
			dynatable.records.updateFromJson({records: items});
			dynatable.records.init();
			dynatable.process();
		});
	};
	
	this.update = function( value ) {
		RPC.call("series", [value], function(items){
			//console.log(items);
			var dynatable = $('#series-table').data('dynatable');
			dynatable.records.updateFromJson({records: items});
			dynatable.records.init();
			dynatable.process();
		});
		
	};	
		
	
	
	
	this.update_details = function ( value ) {
		console.log("Show detailed information for show " + value);
		RPC.call("series_details", [value], function(items){
			var record = items[0];
			var txt = "";
			var img;
			
			$('#seasons_list').empty();
			for(var i = 0; i < record.episodes; i++) {
				if ( i == 0 ) {
					txt = "Specials";
				} else {
					txt = "Season " + i;
				}
				console.log("ADD");
				$('#seasons_list').append('<a class="list-group-item" data-key="'+record.id+'" data-season="'+i+'">' + txt + '</a>');
			}
						
			img = record.poster;
			if ( img != "none.jpg") {
				if (record.poster.charAt(0) == "\\") {
					img = "http://image.tmdb.org/t/p/w154" + record.poster.substring(1);
				} else {
					img = "http://image.tmdb.org/t/p/w154" + record.poster;	
				}			
			}			
			
			$('#series_cover').attr('src', img);
			$('#series_title').text(record.name);
			
			$('#series_overview').text(record.overview);
			
			$('#series_container').attr('style', 'height:720px;background-size:cover;background-image: url("http://image.tmdb.org/t/p/original/'+ record.backdrop +'");')
			
			
			console.log(items);
			
//			<ul class="list-group">
//			  
			
			
		});
	};
	
}(jQuery));

