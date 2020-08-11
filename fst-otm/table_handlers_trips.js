// Таблица с походами
var table;

// Фильтр по дате начала похода
function customFilterByDateStart(data, filterParams){
    //data - the data for the row being filtered
    //filterParams - params object passed to the filter
    var isInFuture = moment(data.start_date, "DD.MM.YYYY") > moment();
    var isInRange = moment(data.start_date, "DD.MM.YYYY").subtract(filterParams.value, filterParams.units) < moment();
    return isInFuture && isInRange;
}

// Фильтр по дате завершения похода
function customFilterByDateFinish(data, filterParams){
    //data - the data for the row being filtered
    //filterParams - params object passed to the filter
    var isInPast = moment(data.finish_date, "DD.MM.YYYY").add(1,'days') < moment();
    var isInRange = moment(data.start_date, "DD.MM.YYYY").add(filterParams.value, filterParams.units) > moment();
    return isInPast && isInRange;
}

// Фильтр по датам похода
function customFilterByDate(data, filterParams){
    //data - the data for the row being filtered
    //filterParams - params object passed to the filter
    switch (filterParams.type) {
        case "past":
            return moment(data.finish_date, "DD.MM.YYYY").add(1,'days') < moment();
        case "present":
            var isStarted = moment(data.start_date, "DD.MM.YYYY") < moment();
            var isFinished = moment(data.finish_date, "DD.MM.YYYY").add(1,'days') < moment();
            return isStarted && !isFinished;
        case "future":
            return moment(data.start_date, "DD.MM.YYYY") > moment();
    }
    return false;
}

// Фильтр по категории сложности
function customFilterByDifficulty(data, filterParams){
    //data - the data for the row being filtered
    //filterParams - params object passed to the filter
    return data.difficulty.startsWith(filterParams.difficulty);
}

// Применить фильтры
function applyFilters() {
    var complex_filter = [];

    // Фильтр по имени
    if (document.getElementById("find_name_str").value != "")
        complex_filter.push({field:"name", type:"like", value:document.getElementById("find_name_str").value});

    // Фильтр по клубу
    if (document.getElementById("find_club_str").value != "")
        complex_filter.push({field:"club", type:"like", value:document.getElementById("find_club_str").value});

    // Фильтр по виду спорта (тип "маршрут")
    if (document.getElementById("sport_type").value != "все")
        complex_filter.push({field:"sport_type", type:"like", value:document.getElementById("sport_type").value});

    // Применение фильтров
    table.setFilter(complex_filter);
    
    // Дополнительная фильтрация по категориям сложности
    if (document.getElementById("difficulty").value != "все") {
        table.addFilter(customFilterByDifficulty, {difficulty:document.getElementById("difficulty").value});
    }

    // Дополнительная фильтрация по датам
    switch ($('#date').val()) {
        case "past":
            table.addFilter(customFilterByDate, {type:"past"});
            break;
        case "present":
            table.addFilter(customFilterByDate, {type:"present"});
            break;
        case "future":
            table.addFilter(customFilterByDate, {type:"future"});
            break;
        case "next1month":
            table.addFilter(customFilterByDateStart, {units:"months", value:1});
            break;
        case "next2month":
            table.addFilter(customFilterByDateStart, {units:"months", value:2});
            break;
        case "next6month":
            table.addFilter(customFilterByDateStart, {units:"months", value:6});
            break;
        case "prev1month":
            table.addFilter(customFilterByDateFinish, {units:"months", value:1});
            break;
        case "prev2month":
            table.addFilter(customFilterByDateFinish, {units:"months", value:2});
            break;
        case "prev6month":
            table.addFilter(customFilterByDateFinish, {units:"months", value:6});
            break;
    }

    // Печать количества записей
    $('#search-results').html("Найдено записей: <strong>" + table.getDataCount(true) + "</strong>");
}

// Очистить все фильтры
function doFilterClear() {
    $('#find_name_str').val('');
    $('#find_club_str').val('');
    $('#sport_type').val('все').selectmenu('refresh');
    $('#difficulty').val('все').selectmenu('refresh');
    $('#date').val('все').selectmenu('refresh');
    applyFilters();
}


$( function() {

    // Создание объекта Tabulator на DOM элементе с идентификатором "full-table"
    table = new Tabulator("#full-table", {
        height:1230,
        data:php_data, // assign data to table
        layout:"fitData", // fit columns to width of table (optional)
        columns:[ // Define Table Columns
            {title:"№ п/п",                           field:"id",          width:50  },
            {title:"№ МК",                            field:"book_id",     width:70  },
            {title:"ФИО руководителя",                field:"name",        width:150 },
            {title:"Организация",                     field:"club",        width:150 },
            {title:"Кол-во человек",                  field:"part_cnt",    width:50  },
            {title:"Вид маршрута",                    field:"sport_type",  width:130 },
            {title:"План. к.с.",                      field:"difficulty",  width:70  },
            {title:"Туристский район",                field:"region",      width:170 },
            {title:"План. старт",                     field:"start_date",  width:100, sorter:"date", sorterParams:{format:"DD.MM.YYYY"},
                formatter: function(cell, formatterParams){
                    var start_date_value = cell.getValue();
                    var finish_date_value = cell.getRow().getData().finish_date;
                    var is_ongoing = (moment(start_date_value, "DD.MM.YYYY") < moment()) && (moment() < moment(finish_date_value, "DD.MM.YYYY").add(1,'days'));
                    if (is_ongoing) {
                      return "<span style='color:green; font-weight:bold;'>" + start_date_value + "</span>";
                    }
                    else if((moment(start_date_value, "DD.MM.YYYY") > moment()) && (moment(start_date_value, "DD.MM.YYYY").subtract(1,'month') < moment())) {
                      return "<span style='color:blue; font-weight:bold;'>" + start_date_value + "</span>";
                    }
                    else {
                      return "<span style='font-weight:bold;'>" + start_date_value + "</span>";
                    }
                }
            },
            {title:"План. финиш",                     field:"finish_date", width:100, sorter:"date", sorterParams:{format:"DD.MM.YYYY"},
                formatter: function(cell, formatterParams){
                    var start_date_value = cell.getRow().getData().start_date;
                    var finish_date_value = cell.getValue();
                    var is_ongoing = (moment(start_date_value, "DD.MM.YYYY") < moment()) && (moment() < moment(finish_date_value, "DD.MM.YYYY").add(1,'days'));
                    if (is_ongoing) {
                      return "<span style='color:green; font-weight:bold;'>" + finish_date_value + "</span>";
                    }
                    else if((moment(finish_date_value, "DD.MM.YYYY").add(1,'days') < moment()) && (moment(finish_date_value, "DD.MM.YYYY").add(1,'days').add(1,'month') > moment())) {
                      return "<span style='color:orange; font-weight:bold;'>" + finish_date_value + "</span>";
                    }
                    else {
                      return "<span style='font-weight:bold;'>" + finish_date_value + "</span>";
                    }
                }
            },
            {title:"Председатель МКК",                field:"mkk_leader",  width:135 },
            {title:"Члены МКК",                       field:"mkk_list",    width:135 },
        ]
    });

    // Выбор вида спорта (тип "маршрут")
    $( "#sport_type" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Выбор планируемой категории сложности
    $( "#difficulty" ).selectmenu({
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

    // Информация о датах создания и модификации
    $( "#modified_date_placeholder" ).text(modified_date);
    $( "#generated_date_placeholder" ).text(generated_date);
});
