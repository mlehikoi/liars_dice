/*global $, console*/
/*eslint no-console: ['error', { allow: ['log'] }] */
///*eslint no-unused-vars: ['error', { 'vars': 'local' }]*/
/*global window, Option */
/*eslint-disable-vars-on-top */

var usrName = ''; // eslint-disable-line no-unused-vars
var userId = '';
var games;
var myGame;

const State = {
    NOT_JOINED: 1,
    JOINED: 2,
    WAITING: 3,
    GAME_ON: 4
};
var myState;
var timer;
var n = 1;
var face = 1;

function show(toShow) {
    const divs = [
        '#welcomeMessage',
        '#Login',
        '#SetupCreate',
        '#WaitGameStart',
        '#GameOn'
    ];
    for (const div of divs)
        if (div != toShow)
            $(div).addClass('hidden');
    $(toShow).removeClass('hidden');
}
function handleState() {
    console.log('handleState ' + myState);
    if (myState == State.WAITING) {
        $('#welcomeMessage').html(usrName + ', waiting for others' +
                                  ' to join the game. Click start when you\'re' +
                                  ' ready to start the game.');
        $('#welcomeMessage').removeClass('hidden');
        $('#Login').addClass('hidden');
        $('#SetupCreate').addClass('hidden');
        $('#WaitGameStart').removeClass('hidden');
        let players = [];
        for (let player of myGame.players) {
            players.push(player.name);
        }
        $('#players-waiting').html('Players: ' + players.join(', '));
        //timer = setTimeout(function(){ getStatus(); }, 1000);
    } else if (myState == State.GAME_ON) {
        console.log('GAME ON');
        show('#GameOn');
    }
}
function join(game) {
    console.log(game);
    $.post('/api/join', JSON.stringify({id: userId, game: game}), function(json) {
        console.log(json);
        getStatus();
    }, 'json');
}

function startGame() {
    console.log('startGame');
    $.post('/api/startGame', JSON.stringify({id: userId}), function(json) {
        if (json.success) getStatus();
    }, 'json');
}

function selectedGame() {
    const selectedGame = $('#games').val();
    for (let game of games) {
        if (game.game == selectedGame) {
            console.log(game);
            let txt = 'Players: ';
            txt += game.players.join(', ');
            $('#players').html(txt);
        }
    }
}
function refreshGames() {
    $.getJSON('/api/games', function (json) {
        games = json;
        let gameSelect = $('#games');
        gameSelect.empty();
        for (let game of games) {
            gameSelect.append(new Option(game.game, game.game));
        }
        selectedGame();
    });
}

function getStatus() {
    $.ajax({
        type: 'POST',
        url: '/api/status',
        data: JSON.stringify({id: userId}),
        contentType: 'application/json; charset=utf-8',
        dataType: 'json',
        success: function (data) {
            if (data.success) {
                if (data.hasOwnProperty('game')) {
                    usrName = data.name;
                    myGame = data.game;
                    if (data.game.state == 'GAME_STARTED') {
                        myState = State.GAME_ON;
                        handleState(myState);
                    } else if (data.game.state == 'GAME_NOT_STARTED') {
                        console.log(data);
                        myState = State.WAITING;
                        $('#welcomeMessage').html(data.name + ', waiting for others' +
                                                  ' to join the game. Click start when you\'re' +
                                                  ' ready to start the game.');
                        $('#welcomeMessage').removeClass('hidden');
                        $('#Login').addClass('hidden');
                        $('#SetupCreate').addClass('hidden');
                        $('#WaitGameStart').removeClass('hidden');
                        handleState();
                    }
                } else {
                    usrName = data.name;
                    $('#welcomeMessage').html('Welcome, ' + data.name + '.' +
                                              ' Start up a new game or select an existing' +
                                              ' game to join.');
                    $('#welcomeMessage').removeClass('hidden');
                    $('#Login').addClass('hidden');
                    $('#SetupCreate').removeClass('hidden');
                    refreshGames();
                }
            } else {
                $('#Login').removeClass('hidden');
                $('#SetupCreate').addClass('hidden');
            }
        },
        error: function () {
            console.log('error');
        }
    });
}

function login(name) { // eslint-disable-line no-unused-vars
    'use strict';
    console.log('Fetching ' + name);
    $.ajax({
        type: 'POST',
        url: '/api/login',
        data: JSON.stringify({'name': name}),
        contentType: 'application/json; charset=utf-8',
        dataType: 'json',
        success: function (data) {
            console.log(data);
            if (data.success) {
                usrName = data.userName;
                window.location.replace('?id=' + data.id);
            } else {
                $('#userNameStatus').html(name + ' is already taken.');
                $('#userNameStatus').removeClass('hidden');
            }
            console.log(data);
        },
        error: function () {
            console.log('error');
        }
    });
}

function createGame(name) { // eslint-disable-line no-unused-vars
    'use strict';
    $.ajax({
        type: 'POST',
        url: '/api/newGame',
        data: JSON.stringify({game: name, id: userId}),
        contentType: 'application/json; charset=utf-8',
        dataType: 'json',
        success: function (data) {
            console.log(data);
            if (data.success) {
                getStatus();
            } else {
                $('#userNameStatus').html(name + ' already exists.');
                $('#userNameStatus').removeClass('hidden');
            }
            console.log(data);
        },
        error: function () {
            //alert(errMsg);
            console.log('error');
            //console.log(errMsg);
        }
    });
}

// HTML hooks
$('#games').on('change', function () {
    selectedGame();
});
$('#join').click(function() {
    join($('#games').val());
});
$('#start-game').click(function() {
    startGame();
});

$(function() {
    $.get('/gameon.html', function (html) {
        $('#GameOn').html(html);
        
        $('#n-minus').click(function() {
            if (n > 0) --n;
            $('#n').html(n);
        });
        $('#n-plus').click(function() {
            ++n;
            $('#n').html(n);
        });
        $('#face-minus').click(function() {
            $('#face-' + face).addClass('hidden');
            --face;
            if (face <= 0) face = 6;
            $('#face-' + face).removeClass('hidden');
        });
        $('#face-plus').click(function() {
            $('#face-' + face).addClass('hidden');
            ++face;
            if (face > 6) face = 1;
            $('#face-' + face).removeClass('hidden');
        });
    });
});


