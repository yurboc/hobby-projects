// Таблица со спортсменами
var table;

// Фильтр по дате присвоения разряда
function customFilterByDateApply(data, filterParams){
    //data - the data for the row being filtered
    //filterParams - params object passed to the filter
    return moment(data.date_apply, "DD.MM.YYYY") >= moment().subtract(filterParams.months, 'months');
}

// Фильтр по дате истечения разряда
function customFilterByDateExpire(data, filterParams){
    //data - the data for the row being filtered
    //filterParams - params object passed to the filter
    var expired = moment(data.date_expire, "DD.MM.YYYY") < moment();
    if (typeof filterParams.filterExpired !== 'undefined') {
        return (filterParams.filterExpired == 1 && expired) || (filterParams.filterExpired == 0 && !expired);
    }
    var showExpired = (filterParams.value == 0) && expired;
    var suitable = moment(data.date_expire, "DD.MM.YYYY") <= moment().add(filterParams.value, filterParams.units);
    var showSuitable = (filterParams.value != 0) && (!expired) && suitable;
    return showExpired || showSuitable;
}

// Применить фильтры
function applyFilters() {
    var complex_filter = [];
    var new_url_data = {};

    // Фильтр по имени
    if (document.getElementById("find_name_str").value != "") {
        complex_filter.push({field:"name", type:"like", value:document.getElementById("find_name_str").value});
        new_url_data["find_name_str"] = document.getElementById("find_name_str").value;
    }

    // Фильтр по клубу
    if (document.getElementById("find_club_str").value != "") {
        complex_filter.push({field:"club", type:"like", value:document.getElementById("find_club_str").value});
        new_url_data["find_club_str"] = document.getElementById("find_club_str").value;
    }

    // Фильтр по программе
    if (document.getElementById("prog").value != "все") {
        complex_filter.push({field:"prog_type", type:"like", value:document.getElementById("prog").value});
        new_url_data["prog"] = document.getElementById("prog").value;
    }

    // Фильтр по виду спорта
    if (document.getElementById("sport").value != "все") {
        complex_filter.push({field:"sport_type", type:"like", value:document.getElementById("sport").value});
        new_url_data["sport"] = document.getElementById("sport").value;
    }

    // Фильтр по разрядам
    if (document.getElementById("rank").value == "Юношеские") {
        complex_filter.push({field:"rank", type:"like", value:"ю разряд"});
        new_url_data["rank"] = document.getElementById("rank").value;
    }
    else if (document.getElementById("rank").value != "все") {
        complex_filter.push({field:"rank", type:"=", value:document.getElementById("rank").value});
        new_url_data["rank"] = document.getElementById("rank").value;
    }

    // Применение фильтров
    table.setFilter(complex_filter);

    // Дополнительная фильтрация по датам
    switch ($('#date').val()) {
        case "last1month":
            table.addFilter(customFilterByDateApply, {months:1});
            break;
        case "last2month":
            table.addFilter(customFilterByDateApply, {months:2});
            break;
        case "last6month":
            table.addFilter(customFilterByDateApply, {months:6});
            break;
        case "expNo":
            table.addFilter(customFilterByDateExpire, {filterExpired:0});
            break;
        case "expYes":
            table.addFilter(customFilterByDateExpire, {filterExpired:1});
            break;
        case "exp1month":
            table.addFilter(customFilterByDateExpire, {units:"weeks", value:4});
            break;
        case "exp3month":
            table.addFilter(customFilterByDateExpire, {units:"months", value:3});
            break;
    }
    if ($('#date').val() != "все") {
        new_url_data["date"] = $('#date').val();
    }

    // Печать количества записей
    $('#search-results').html("Найдено записей: <strong>" + table.getDataCount('active') + "</strong>");

    // Обновление строки URL
    const searchParams = new URLSearchParams(new_url_data);
    history.replaceState(null, document.title, window.location.pathname + (Object.keys(new_url_data).length ? "?" : "") + searchParams)
}

// Очистить все фильтры
function doFilterClear() {
    $('#find_name_str').val('');
    $('#find_club_str').val('');
    $('#prog').val('все').selectmenu('refresh');
    $('#sport').val('все').selectmenu('refresh');
    $('#rank').val('все').selectmenu('refresh');
    $('#date').val('все').selectmenu('refresh');
    applyFilters();
}


$( function() {

    // Создание объекта Tabulator на DOM элементе с идентификатором "full-table"
    table = new Tabulator("#full-table", {
        height:1230,
        tooltips:true,
        data:php_data, // assign data to table
        layout:"fitData", // fit columns to width of table (optional)
        columns:[ // Define Table Columns
            {title:"№ п/п", field:"id", width:50, tooltip: false},
            {title:"ФИО спортсмена", field:"name", width:300},
            {title:"Спортивная организация, турклуб", field:"club", width:150},
            {title:"Спортивный разряд/ звание", field:"rank", width:100},
            {title:"Дата присвоения", field:"date_apply", width:100, sorter:"date", sorterParams:{format:"DD.MM.YYYY"}},
            {title:"№ Приказа/ Распоряжения о присвоении", width:150, field:"order_no",
                formatter: function(cell, formatterParams){
                    var order_no_value = cell.getRow().getData().order_no;
                    var order_link_value = cell.getRow().getData().order_link;
                    if (order_link_value.startsWith("http")) {
                        if (order_no_value == "") {
                            order_no_value = "ссылка";
                        }
                        return "<i class='ui-icon ui-icon-extlink'></i> <a style='color:blue' target='_blank' href='" + order_link_value + "'>" + order_no_value + "</a>";
                    }
                    else {
                        return (order_no_value == "") ? "" : "<i class='ui-icon ui-icon-document'></i> " + order_no_value;
                    }
                },
                tooltip: function(cell){
                    //function should return a string for the tooltip of false to hide the tooltip
                    //return cell.getColumn().getField() + " - " + cell.getValue(); //return cells "field - value";
                    var order_no_value = cell.getRow().getData().order_no;
                    var order_link_value = cell.getRow().getData().order_link;
                    var tooltip_text = (order_no_value == "") ? "Номер отсутствует" : order_no_value;
                    if (order_link_value.startsWith("http")) {
                        tooltip_text += "\nСсылка: " + order_link_value;
                    }
                    else if (order_link_value != "") {
                        tooltip_text += "\nИнфо: " + order_link_value;
                    }
                    return tooltip_text;
                }
            },
            {title:"статус разряда/ звания", field:"rank_state", width:120},
            {title:"разряд действует до", field:"date_expire", width:110, sorter:"date", sorterParams:{format:"DD.MM.YYYY"},
                formatter: function(cell, formatterParams){
                    var cell_value = cell.getValue();
                    if(moment(cell_value, "DD.MM.YYYY") <= moment()) {
                      return "<span style='color:red; font-weight:bold;'>" + cell_value + "</span>";
                    }
                    else if(moment(cell_value, "DD.MM.YYYY") <= moment().add(4, 'weeks')) {
                      return "<span style='color:orange; font-weight:bold;'>" + cell_value + "</span>";
                    }
                    else if(moment(cell_value, "DD.MM.YYYY") <= moment().add(3, 'months')) {
                      return "<span style='color:teal; font-weight:bold;'>" + cell_value + "</span>";
                    }
                    else {
                      return "<span style='font-weight:bold;'>" + cell_value + "</span>";
                    }
                }
            },
            {title:"дистанция/ маршрут", field:"prog_type", width:100},
            {title:"вид", field:"sport_type", width:130},
            {title:"ссылка на приказ/распоряжение", field:"order_link", width:135, visible: false, download: true},
        ]
    });

    // Применение URL-параметров
    if (window.location.search.length) {
        const urlParams = new URLSearchParams(window.location.search);
        urlParams.forEach(function(value, key) {
            $("#"+key).val(value);
        });
        applyFilters();
    }

    // Выбор программы дистанция/маршрут
    $( "#prog" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Выбор вида спорта
    $( "#sport" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Выбор разрядов и званий
    $( "#rank" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Выбор фильтра по датам
    $( "#date" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Кнопки "Найти" и "Сброс"
    $( "#find_btn" ).button().click(applyFilters);
    $( "#clear_btn" ).button().click(doFilterClear);

    // Информация о табличке
    $( "#dialog_about" ).dialog({
        autoOpen: false,
        width: 495,
        modal: true,
        buttons: {
            OK: function() {
                $( this ).dialog( "close" );
            }
        },
        position: {
            my: "center top",
            at: "center top",
            of: "#full-table", // of: window, // при window диалог съезжал вниз
            collision: "none"
        },
        //create: function (event, ui) { // чтобы диалог не двигался при прокрутке
        //  $(event.target).parent().css('position', 'fixed');
        //}
    });
    $( "#show_info_btn" ).button( {
        icon: "ui-icon-info",
        showLabel: false
      } ).click(function() {
        $('#dialog_about').dialog('open');
    });

    // Кнопка "Экспорт в XLSX"
    $("#xlsx_btn").button().click(function(){
        table.download("xlsx", "sportsmen.xlsx", {sheetName:"Спортсмены"});
    });

    // Информация о датах создания и модификации
    $( "#modified_date_placeholder" ).text(modified_date);
    $( "#generated_date_placeholder" ).text(generated_date);
});
