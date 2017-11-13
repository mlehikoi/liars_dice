/*global $, console*/
/*eslint no-console: ['error', { allow: ['log'] }] */
///*eslint no-unused-vars: ['error', { 'vars': 'local' }]*/
/*global window, Option */
/*eslint-disable-vars-on-top */

var myName = ''; // eslint-disable-line no-unused-vars
var myId = '';
var games;
var myGame;
//var prevStatusTxt = '';
var prevBid = {
    n: 0,
    face: 1
};
var myBid = {
    n: 1,
    face: 1
};

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
var diceImages = [];
var bidImages = [];

var myState;
var timer;

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
        'welcomeMessage',
        'Login',
        'SetupCreate',
        'WaitGameStart',
        'GameOn',
        'GameStarted',
        'start-round',
        'InvalidId'
    ];
    for (const div of divs)
        if (toShow.indexOf(div) == -1)
            $('#' + div).addClass('hidden');
    for (const div of toShow)
        $('#' + div).removeClass('hidden');
}

function score(bid) {
    return bid.face == 6 ? 20 * bid.n : 10 * bid.n + bid.face;
}

// Draw the number of dice of bid
// Make sure the bid is high enough
function drawMyBid() {
    // Make sure bid is high enough
    while (score(myBid) <= score(prevBid)) ++myBid.n;
    
    $('#n').html(myBid.n);
    let lowerBid = {n: myBid.n - 1, face: myBid.face };
    if (score(lowerBid) <= score(prevBid)) {
        $('#n-minus').addClass('disabled');    
    }
    else {
        $('#n-minus').removeClass('disabled');
    }
    for (let i = 1; i <= 6; ++i)
        if (i != myBid.face)
            $('#face-' + i).addClass('hidden');
    $('#face-' + myBid.face).removeClass('hidden');
}

function drawDice(pid, cell) {
    let images = '';
    for (let dice of myGame.players[pid].hand) {
        images += '<img src="/images/' + Images[dice][0] + '-24x24.png" alt="' + Images[dice][1] + '" width="24" height="24">\n';
    }
    const adj = myGame.players[pid].adjustment;
    if (adj) {
        images += '<br><span class="label label-default">' + adj + '</span>';
    }
    if (images != diceImages[pid]) {
        diceImages[pid] = images;
        cell.html(images);
    }
    
}

function drawBid(pid, cell) {
    let txt = '';
    const theBid = myGame.players[pid].bid;
    console.log(theBid);
    if (theBid && theBid.n > 0 && theBid.face > 0) {
        txt += theBid.n + ' ';
        txt += '<img src="/images/' + Images[theBid.face][0] + '-24x24.png" alt="' + Images[theBid.face][1] +
            '" width="24" height="24">\n';
    }
    if (txt != bidImages[pid]) {
        bidImages[pid] = txt;
        cell.html(txt);
    }
    
}

function pollStatus() {
    clearTimeout(timer);
    timer = setTimeout(function(){ getStatus(); }, 10000);
}

function handleState() {
    console.log('handleState ' + myState);
    if (myState == State.WAITING) {
        $('#welcomeMessage').html(myName + ', waiting for others' +
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
        if (players.length > 1) {
            $('#start-game').removeClass('hidden');
            $('#start-game-status').addClass('hidden');
        } else {
            $('#start-game').addClass('hidden');
        }
        $('#players-waiting').html('Players: ' + players.join(', '));
        pollStatus();
    } else if (myState == State.GAME_ON) {
        console.log(myGame);
        if (myGame.state == 'GAME_STARTED') {
            // This is dead code.
            
            let txt = '';
            txt += 'Game "' + myGame.game + '" started. Waiting for ';
            const who = myGame.players[myGame.turn].name;
            const myTurn = who == myName;
            txt += myTurn ? 'you' : who;
            txt += ' to start the round.';
            $('#game-started-message').html(txt);
            if (myTurn) show(['GameStarted', 'start-round']); else show('GameStarted');
        }
        else {
            const bid = myGame.bid;
            if (bid) {
                prevBid.n = bid.n;
                prevBid.face = bid.face >= 1 && bid.face <= 6 ? bid.face : 1;
            } else {
                prevBid.n = 0;
                prevBid.face = 1;
            }
            myBid = { n: prevBid.n, face: prevBid.face };
            drawMyBid();
            // ROUND_STARTED
            // CHALLENGE
            console.log(myGame.state);
            // If first time
            let table = document.getElementById('player-table');
            let numPlayers = myGame.players.length;
            let winners = [];
            let losers = [];
            let diceLost = 0;
            for (let i = 0; i < table.rows.length; ++i) {
                if (i == 0) continue;
                let pid = i - 1;
                if (pid < numPlayers) {
                    const fullname = myGame.players[pid].name;
                    if (pid == myGame.turn) $(table.rows[i]).addClass('active'); else $(table.rows[i]).removeClass('active');
                    let name = splitString(myGame.players[pid].name, 10);
                    if (name == myName) name = '<strong>' + name + '</strong>';
                    if (myGame.players[pid].winner) {
                        winners.push(fullname);
                        name = '<span class="glyphicon glyphicon-thumbs-up"></span> ' + name;
                    }
                    else if (myGame.players[pid].loser) {
                        losers.push(fullname);
                        diceLost = myGame.players[pid].adjustment;
                        name = '<span class="glyphicon glyphicon-thumbs-down"></span> ' + name;
                    } 
                    else if (pid == myGame.turn) name = '<span class="glyphicon glyphicon-play"></span>' + name;
                    $(table.rows[i].cells[0]).html(name);
                    drawDice(pid, $(table.rows[i].cells[1]));
                    drawBid(pid, $(table.rows[i].cells[2]));
                    $(table.rows[i]).removeClass('hidden');
                } else {
                    $(table.rows[i]).addClass('hidden');
                }
            }
            if (myGame.state == 'ROUND_STARTED') {
                const playerInTurn = myGame.players[myGame.turn].name;
                if (myGame.players[myGame.turn].name == myName) {
                    $('#game-msg').html(myName + ', it\'s your turn. You can either make a higher bid or challeng the bid.');
                    if (prevBid.n) $('#challenge').removeClass('disabled');
                    else           $('#challenge').addClass('disabled');
                    $('#BidOrChallenge').removeClass('hidden');
                }
                else {
                    $('#game-msg').html('Waiting for ' + playerInTurn + ' to bid or challenge.');
                    $('#BidOrChallenge').addClass('hidden');
                    pollStatus();
                }
                $('#DoneViewingResults').addClass('hidden');
            }
            else if (myGame.state == 'CHALLENGE') {
                $('#game-msg').html('Round ended. ' + winners.join(', ') + ' won. ' +
                                   losers.join(', ') + ' lost ' + -diceLost + ' dice.<br>' +
                                   'Click done to start new round.');
                $('#BidOrChallenge').addClass('hidden');
                $('#DoneViewingResults').removeClass('hidden');
                // Not polling status here. Player will start the round when he or she is ready.
            }
            else if (myGame.state == 'GAME_FINISHED') {
                $('#game-msg').html('Game ended. Winner is <strong>' + winners.join(', ') + '</strong>.<br>' +
                                   'Click done to start new round.');
                $('#BidOrChallenge').addClass('hidden');
                $('#DoneViewingResults').removeClass('hidden');
            }
            show('GameOn');
        }
    }
}
function join(game) {
    $('#join-status').addClass('hidden');
    $('#create-status').addClass('hidden');
    $.post('/api/join', JSON.stringify({id: myId, game: game}), function(json) {
        console.log(json);
        if (json.success) {
            getStatus();    
        }
        else {
            $('#join-status').html(json.error);
            $('#join-status').removeClass('hidden');
        }
        
    }, 'json');
}

function createGame(name) {
    $('#join-status').addClass('hidden');
    $('#create-status').addClass('hidden');
    $('#join-status').addClass('hidden');
    $('#create-status').addClass('hidden');
    $.post('/api/newGame', JSON.stringify({id: myId, game: name}), function(json) {
        console.log(json);
        if (json.success) {
            getStatus();    
        }
        else {
            $('#create-status').html(json.error);
            $('#create-status').removeClass('hidden');
        }
        
    }, 'json');
}

function startGame() {
    console.log('startGame');
    $.post('/api/startGame', JSON.stringify({id: myId}), function(json) {
        console.log(json);
        if (json.success) {
            getStatus();
        }
        else {
            $('#start-game-status').html(json.error);
            $('#start-game-status').removeClass('hidden');
        }
    }, 'json');
}

function startRound() {
    console.log('startRound');
    $.post('/api/startRound', JSON.stringify({id: myId}), function(json) {
        console.log(json);
        getStatus();
    }, 'json');
}

function bid() {
    console.log('bid');
    $.post('/api/bid', JSON.stringify({id: myId, n: myBid.n, face: myBid.face}), function(json) {
        console.log(json);
        if (json.success) getStatus();
    }, 'json');
}

function challenge() {
    console.log('challenge');
    $.post('/api/challenge', JSON.stringify({id: myId}), function(json) {
        console.log(json);
        if (json.success) getStatus();
    }, 'json');
}

function done() {
    if (myGame.state == 'GAME_FINISHED')
        startGame();
    else
        startRound();
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
    $.post('/api/status', JSON.stringify({id: myId}), function (json) {
        if (json.success) {
            console.log(json);
            if (json.hasOwnProperty('game')) {
                myName = json.name;
                myGame = json.game;
                if (myGame.state != 'GAME_NOT_STARTED') {
                    myState = State.GAME_ON;
                    handleState(myState);
                } else if (myGame.state == 'GAME_NOT_STARTED') {
                    myState = State.WAITING;
                    $('#welcomeMessage').html(json.name + ', waiting for others' +
                                              ' to join the game. Click start when you\'re' +
                                              ' ready to start the game.');
                    $('#welcomeMessage').removeClass('hidden');
                    $('#Login').addClass('hidden');
                    $('#SetupCreate').addClass('hidden');
                    $('#WaitGameStart').removeClass('hidden');
                    handleState();
                }
            } else {
                myName = json.name;
                $('#welcomeMessage').html('Welcome, ' + json.name + '.' +
                                          ' You can select an existing game below and join' +
                                          ' it. You can click the refresh' +
                                          ' button on the right side of games to see newly' +
                                          ' created games.');
                $('#welcomeMessage').removeClass('hidden');
                $('#Login').addClass('hidden');
                $('#SetupCreate').removeClass('hidden');
                refreshGames();
            }
        } else {
            console.log(json);
            if (json.error == 'NO_PLAYER') {
                show('InvalidId');
            }
        }
    }, 'json').fail(function(response) {
        alert('Error: ' + response.responseText);
    });
}

function adjustN(offset) {
    myBid.n += offset;
    drawMyBid();
}

function adjustFace(offset) {
    const face = myBid.face + offset;
    myBid.face = face < 1 ? 6 : face > 6 ? 1 : face; 
    
    drawMyBid();
}

// HTML hooks
$(function() {
    $('#games').on('change', function () {
        selectedGame();
    });
    $('#join').click(function() {
        join($('#games').val());
    });
    $('#create').click(function () {
        createGame($('#gameName').val());
    });
    $('#start-game').click(function() {
        startGame();
    });
    $('#start-round').click(function() {
        startRound();
    });
    $('#n-minus').click(function() {
        adjustN(-1);
    });
    $('#n-plus').click(function() {
        adjustN(1);
    });
    $('#face-minus').click(function() {
        adjustFace(-1);
    });
    $('#face-plus').click(function() {
        adjustFace(1);
    });
    $('#bid').click(function() {
        bid();
    });
    $('#challenge').click(function() {
        challenge();
    });
    $('#done-viewing-results').click(function() {
        done();
    });
    myId = getParameterByName('id');
    console.log('Loaded content for ' + myId);
    getStatus();
});


