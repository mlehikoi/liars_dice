/*global
browser:true,
$, JQuery:true,
alert, confirm, console, Debug, opera, prompt, WSH
*/

var userKey = "";

function login(name) {
    'use strict';
    alert(name);
    $.ajax({
        type: "POST",
        url: "/api/login",
        data: JSON.stringify({"name": name}),
        contentType: "application/json; charset=utf-8",
        dataType: "json",
        success: function (data) {
            alert(data);
        },
        failure: function (errMsg) {
            alert(errMsg);
        }
    });
}