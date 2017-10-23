/*global $, console*/
/*eslint no-console: ["error", { allow: ["log"] }] */
///*eslint no-unused-vars: ["error", { "vars": "local" }]*/
/*global window*/

var userName = "";
var userId = "";

function getStatus() {
    'use strict';
    console.log(userId);
    $.ajax({
        type: "POST",
        url: "/api/status",
        data: JSON.stringify({id: userId}),
        contentType: "application/json; charset=utf-8",
        dataType: "json",
        success: function (data) {
            console.log(data);
            if (data.success) {
                if (data.hasOwnProperty("game")) {
                    console.log(data.game);
                    $("#welcomeMessage").html(data.name + ", waiting for others to join the game. Click start when you're" +
                                              " ready to start the game.");
                    $("#welcomeMessage").removeClass("hidden");
                    $("#Login").addClass('hidden');
                    $("#SetupCreate").addClass('hidden');
                } else {
                    userName = data.name;
                    $("#welcomeMessage").html("Welcome, " + data.name + "." +
                                              " Start up a new game or select an existing" +
                                              " game to join.");
                    $("#welcomeMessage").removeClass("hidden");
                    $("#Login").addClass('hidden');
                    $("#SetupCreate").removeClass('hidden');
                }
            } else {
                $("#Login").removeClass('hidden');
                $("#SetupCreate").addClass('hidden');
            }
        },
        error: function () {
            console.log("error");
        }
    });
}

function login(name) { // eslint-disable-line no-unused-vars
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
                userName = data.userName;
                window.location.replace("?id=" + data.id);
            } else {
                $("#userNameStatus").html(name + " is already taken.");
                $("#userNameStatus").removeClass('hidden');
            }
            console.log(data);
        },
        error: function () {
            //alert(errMsg);
            console.log("error");
            //console.log(errMsg);
        }
    });
}

function createGame(name) { // eslint-disable-line no-unused-vars
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
                getStatus();
            } else {
                $("#userNameStatus").html(name + " already exists.");
                $("#userNameStatus").removeClass('hidden');
            }
            console.log(data);
        },
        error: function () {
            //alert(errMsg);
            console.log("error");
            //console.log(errMsg);
        }
    });
}

