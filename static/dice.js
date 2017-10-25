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
const Images = [
    ['question', '?'],
    ['one', '1'],
    ['two', '2'],
    ['three', '3'],
    ['four', '4'],
    ['five', '5'],
    ['star', '6'],
];

var myState;
var timer;
var n = 1;
var face = 1;

function getParameterByName(name, url) {
    if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, '\\$&');
    var regex = new RegExp('[?&]' + name + '(=([^&#]*)|&|#|$)'),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, ' '));
}

function splitString(str, len) {
    let lines = [];
    let line = '';
    for (let c of str) {
        if (c == ' ') {
            lines.push(line);
            line = '';
        }
        else {
            line += c;
            if (line.length >= len) {
                lines.push(line);
                line = '';
            }
        }
    }
    lines.push(line);
    return lines.join(' ');
}

function show(toShow) {
    if (!Array.isArray(toShow)) toShow = [toShow];
    console.log(toShow);
    const divs = [
        '#welcomeMessage',
        '#Login',
        '#SetupCreate',
        '#WaitGameStart',
        '#GameOn',
        '#GameStarted',
        '#start-round'
    ];
    for (const div of divs)
        if (toShow.indexOf(div) == -1)
            $(div).addClass('hidden');
    for (const div of toShow)
        $(div).removeClass('hidden');
}

function drawDice(pid, cell) {
    let images = '';
    for (let dice of myGame.players[pid].hand) {
        images += '<img src="' + Images[dice][0] + '-512x512.png" alt="' + Images[dice][1] + '" width="24" height="24">\n';
    }
    cell.html(images);
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
        console.log(myGame);
        if (myGame.state == 'GAME_STARTED') {
            let txt = '';
            txt += 'Game "' + myGame.game + '" started. Waiting for ';
            const who = myGame.players[myGame.turn].name;
            const myTurn = who == usrName;
            txt += myTurn ? 'you' : who;
            txt += ' to start the round.';
            $('#game-started-message').html(txt);
            if (myTurn) show(['#GameStarted', "#start-round"]); else show('#GameStarted');
        }
        else {
            // If first time
            let table = document.getElementById('player-table');
            let numPlayers = myGame.players.length;
            for (let i = 0; i < table.rows.length; ++i) {
                if (i == 0) continue;
                let pid = i - 1;
                if (pid < numPlayers) {
                    if (pid == 0) $(table.rows[i]).addClass('active'); else $(table.rows[i]).removeClass('active');
                    $(table.rows[i].cells[0]).html(splitString(myGame.players[pid].name, 10));
                    drawDice(pid, $(table.rows[i].cells[1]));
                    $(table.rows[i]).removeClass('hidden');
                } else {
                    $(table.rows[i]).addClass('hidden');
                }
            }
            show('#GameOn');
        }
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

function startRound() {
    console.log('startRound');
    $.post('/api/startRound', JSON.stringify({id: userId}), function(json) {
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
    console.log('getStatus');
    $.ajax({
        type: 'POST',
        url: '/api/status',
        data: JSON.stringify({id: userId}),
        contentType: 'application/json; charset=utf-8',
        dataType: 'json',
        success: function (data) {
            if (data.success) {
                console.log(data);
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
$('#start-round').click(function() {
    startRound();
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
        userId = getParameterByName('id');
        console.log('Loaded content for ' + userId);
        getStatus();

    });
});


