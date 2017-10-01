/*global
browser:true,
$, JQuery:true,
alert, confirm, console, Debug, opera, prompt, WSH
*/

var userKey = "";
var userName = "";
var userId = "";

function login(name) {
    'use strict';
    console.log("Fetching " + name);
    $.ajax({
        type: "POST",
        url: "/api/login",
        data: JSON.stringify({"name": name}),
        contentType: "application/json; charset=utf-8",
        dataType: "json",
        success: function (data) {
            console.log(data);
            if (data.success) {
                $("#welcomeMessage").html("Welcome, " + name + "." +
                                          " Start up a new game or select an existing" +
                                          " game to join.");
                $("#welcomeMessage").removeClass("hidden");
                $("#Login").addClass('hidden');
                $("#SetupCreate").removeClass('hidden');
                window.location.replace("?id=" + data.playerId);
            } else {
                $("#userNameStatus").html(name + " is already taken.");
                $("#userNameStatus").removeClass('hidden');
            }
            console.log(data);
        },
        error: function (xhr, statusText, err) {
            //alert(errMsg);
            console.log("error");
            //console.log(errMsg);
        }
    });
}

function createGame(name) {
    'use strict';
    $.ajax({
        type: "POST",
        url: "/api/newGame",
        data: JSON.stringify({game: name, id: userId}),
        contentType: "application/json; charset=utf-8",
        dataType: "json",
        success: function (data) {
            console.log(data);
            if (data.success) {
                //@TODO
            } else {
                $("#userNameStatus").html(name + " already exists.");
                $("#userNameStatus").removeClass('hidden');
            }
            console.log(data);
        },
        error: function (xhr, statusText, err) {
            //alert(errMsg);
            console.log("error");
            //console.log(errMsg);
        }
    });
}