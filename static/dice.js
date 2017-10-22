///*global
//browser:true,
//$, JQuery:true,
//alert, confirm, console, Debug, opera, prompt, WSH
//*/
/*jslint devel: true */

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
                //@TODO Is the following necessary? The URL is changed anyway.
//                $("#welcomeMessage").html("Welcome, " + name + "." +
//                                          " Start up a new game or select an existing" +
//                                          " game to join.");
//                $("#welcomeMessage").removeClass("hidden");
//                $("#Login").addClass('hidden');
//                $("#SetupCreate").removeClass('hidden');
                window.location.replace("?id=" + data.id);
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
                getStatus();
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

function getStatus()
{
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
                }
                else {
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
        error: function (xhr, statusText, err) {
            //alert(errMsg);
            console.log("error");
            //console.log(errMsg);
        }
    });
}