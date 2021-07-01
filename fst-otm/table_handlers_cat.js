// Таблица со спортивными разрядами
var table;

// Применить фильтры
function applyFilters() {
    var complex_filter = [];
    var new_url_data = {};

    // Фильтр по имени
    if (document.getElementById("find_name_str").value != "") {
        complex_filter.push({field:"name", type:"like", value:document.getElementById("find_name_str").value});
        new_url_data["find_name_str"] = document.getElementById("find_name_str").value;
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
    applyFilters();
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
            {title:"Статус<br/>рассмотрения<br/>документов", field:"review_state", width:100},
            {title:"Дата<br/>передачи документов<br/>в Москомспорт/Минспорт/ЦФКиС/ЦСТиСК", field:"date_trans_doc", width:100},
            {title:"Приказ/<br/>Распоряжение", width:100, field:"order_no"},
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

    // Выбор вида программы "дистанция"
    $( "#dist_sport" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Выбор вида программы "маршрут"
    $( "#route_sport" ).selectmenu({
        change: function( event, data ) {
            // Значение можно достать из data.item.value
            applyFilters();
        }
    });

    // Выбор категорий
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
        table.download("xlsx", "sport_categories.xlsx", {sheetName:"Спортивные разряды"});
    });

    // Информация о датах создания и модификации
    $( "#modified_date_placeholder" ).text(modified_date);
    $( "#generated_date_placeholder" ).text(generated_date);
});
