// Таблица со спортивными разрядами
var table;

// Расцветка ячеек по значению
var cellColors = {
    'Всего на рассмотрении'            : null,
    'на рассмотрении в Москомспорте'   : null,
    'документы на присвоении в ЦСТиСК' : "#ff9900",
    'на рассмотрении'                  : null,
    'представления на оформлении'      : "#e0f7fa",
    'разрядные книжки на оформлении, разряд присвоен' : "#e0f7fa",
    'документы готовы к выдаче'        : "#ffff00",
    'отказано в присвоении'            : null,
    'нужен скан разрядной книжки'      : "#ff00ff",
    'возврат на доработку'             : null,
    'проверка КС'                      : null,
    'на рассмотрении в Минспорте РФ'   : "#00ff00"
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

    // Фильтр по дисциплине (по программе)
    if (document.getElementById("prog").value != "все") {
        complex_filter.push({field:"prog_type", type:"like", value:document.getElementById("prog").value});
        new_url_data["prog"] = document.getElementById("prog").value;
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

    // Фильтр по статусам рассмотения
    if (document.getElementById("review_state").value != "все") {
        complex_filter.push({field:"review_state", type:"=", value:document.getElementById("review_state").value});
        new_url_data["review_state"] = document.getElementById("review_state").value;
    }

    // Применение фильтров
    table.setFilter(complex_filter);

    // Печать количества записей
    $('#search-results').html("Найдено записей: <strong>" + table.getDataCount('active') + "</strong>");

    // Обновление строки URL
    const searchParams = new URLSearchParams(new_url_data);
    history.replaceState(null, document.title, window.location.pathname + (Object.keys(new_url_data).length ? "?" : "") + searchParams)
}

// Очистить все фильтры
function doFilterClear() {
    $('#find_name_str').val('');
    $('#prog').val('все').selectmenu('refresh');
    $('#rank').val('все').selectmenu('refresh');
    $('#review_state').val('все').selectmenu('refresh');
    applyFilters();
}

// Подсчёт статистики
function calculateStat() {
    var outData = "";
    var states = {
        'Всего на рассмотрении' : 0,
        'на рассмотрении в Москомспорте' : 0,
        'документы на присвоении в ЦСТиСК' : 0,
        'на рассмотрении' : 0,
        'представления на оформлении' : 0,
        'разрядные книжки на оформлении, разряд присвоен' : 0,
        'документы готовы к выдаче' : 0,
        'отказано в присвоении' : 0,
        'нужен скан разрядной книжки' : 0,
        'возврат на доработку' : 0,
        'проверка КС' : 0,
        'на рассмотрении в Минспорте РФ' : 0
    };
    php_data.forEach(function(item, i, arr) {
        states['Всего на рассмотрении'] += 1;
        states[item['review_state']] = ((typeof states[item['review_state']] !== 'undefined')) ? (states[item['review_state']] + 1) : 1;
    });
    outData += "<table border=1 cellspacing=0 cellpadding=5 width=475>";
    Object.keys(states).forEach(function(item, i, arr) {
        cellColor = cellColors[item];
        outData += "<tr><td align=center>" + states[item] + "</td>";
        if (typeof cellColor !== 'undefined' && cellColor != null) {
            outData += "<td bgcolor='" + cellColor + "'>" + item + "</td>";
        }
        else {
            outData += "<td>" + item + "</td>";
        }
        outData += "</tr>";
    });
    outData += "</table>";
    $("#stat_placeholder").html(outData);
}

/*
          'id'             => array_key_exists( 0, $row) ? $row[ 0] : "",
          'name'           => array_key_exists( 1, $row) ? $row[ 1] : "",
          'date_apply'     => array_key_exists( 2, $row) ? $row[ 2] : "",
          'prog_type'      => array_key_exists( 3, $row) ? $row[ 3] : "",
          'rank'           => array_key_exists( 4, $row) ? $row[ 4] : "",
          'review_state'   => array_key_exists( 5, $row) ? $row[ 5] : "",
          'date_trans_doc' => array_key_exists( 6, $row) ? $row[ 6] : "",
          'order_no'       => array_key_exists( 7, $row) ? $row[ 7] : "",
          'req_app_date'   => array_key_exists( 8, $row) ? $row[ 8] : "",
*/

$( function() {
    // Создание объекта Tabulator на DOM элементе с идентификатором "full-table"
    table = new Tabulator("#full-table", {
        height:1230,
        tooltips:true,
        data:php_data, // assign data to table
        layout:"fitData", // fit columns to width of table (optional)
        columns:[ // Define Table Columns
            {title:"№<br/>п/п", field:"id", width:50, tooltip: false},
            {title:"ФИО<br/>спортсмена", field:"name", width:150},
            {title:"Дата<br/>подачи документов<br/>в ФСТ-ОТМ", field:"date_apply", width:100},
            {title:"Дисциплина", field:"prog_type", width:100},
            {title:"Спортивный<br/>разряд/звание", field:"rank", width:100},
            {title:"Статус<br/>рассмотрения<br/>документов", field:"review_state", width:100,
                formatter: function(cell, formatterParams) {
                    var state_value = cell.getValue();
                    cell_color = cellColors[state_value]
                    if (typeof cell_color !== 'undefined' && cell_color != null) {
                        cell.getElement().style.backgroundColor = String(cell_color);
                    }
                    return state_value;
                },
            },
            {title:"Дата<br/>передачи документов<br/>в Москомспорт/Минспорт/ЦФКиС/ЦСТиСК", field:"date_trans_doc", width:100},
            {title:"Приказ/<br/>Распоряжение", width:170, field:"order_no",
                formatter: function(cell, formatterParams){
                    order_no_value = cell.getValue();
                    if (order_no_value.startsWith("http")) {
                        return "<a style='color:blue' target='_blank' href='" + order_no_value + "'>" + order_no_value + "</a>";
                    }
                    else {
                        return (order_no_value == "") ? "" : order_no_value;
                    }
                },
            },
            {title:"Дата<br/>присвоения", field:"req_app_date", width:100},
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

    // Дисциплина
    $( "#prog" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Разряды и звания
    $( "#rank" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Статус рассмотрения документов
    $( "#review_state" ).selectmenu({
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

    // Статистика
    $( "#dialog_stat" ).dialog({
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
    $( "#show_stat_btn" ).button( {
        icon: "ui-icon-calculator",
        showLabel: true
      } ).click(function() {
        calculateStat();
        $('#dialog_stat').dialog('open');
    });

    // Кнопка "Экспорт в XLSX"
    $("#xlsx_btn").button().click(function(){
        table.download("xlsx", "sport_categories.xlsx", {sheetName:"Спортивные разряды"});
    });

    // Информация о датах создания и модификации
    $( "#modified_date_placeholder" ).text(modified_date);
    $( "#generated_date_placeholder" ).text(generated_date);
});
