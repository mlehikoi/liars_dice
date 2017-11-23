(function () {
    'use strict';

    var my = {
            id: '',
            name: '',
            hash: undefined,
            availableGames: undefined,
            timer: undefined,
            bid: {
                n: 1,
                face: 1
            }
        },
        myGame;


    const images = [];
    (function() {
        const metaImages = [
            ['question', '?'],
            ['one', '1'],
            ['two', '2'],
            ['three', '3'],
            ['four', '4'],
            ['five', '5'],
            ['star', '6']
        ];
        for (const meta of metaImages) {
            const image = document.createElement('img');
            image.setAttribute('src', '/images/' + meta[0] + '-24x24.png');
            image.setAttribute('alt', meta[1]);
            image.setAttribute('height', '16');
            image.setAttribute('width', '16');
            images.push(image.cloneNode(true));
        }
    })();

    function getParameterByName(name, url) {
        if (!url) url = window.location.href;
        name = name.replace(/[q[\]]/g, '\\$&');
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
        const bid = myGame.bid;
        const prevBid = {
            n: bid ? bid.n : 0,
            face: bid && bid.face ? bid.face : 1
        };
        // Make sure bid is high enough
        while (score(my.bid) <= score(prevBid)) ++my.bid.n;

        $('#n').html(my.bid.n);
        if (score({ n: my.bid.n - 1, face: my.bid.face }) <= score(prevBid)) {
            $('#n-minus').addClass('disabled');    
        } else {
            $('#n-minus').removeClass('disabled');
        }
        for (let i = 1; i <= 6; ++i)
            if (i != my.bid.face)
                $('#face-' + i).addClass('hidden');
        $('#face-' + my.bid.face).removeClass('hidden');
    }

    /**
     * Remove all children from node
     * @param {HTMLElement} node 
     */
    function removeChildren(node) {
        while (node.firstChild) {
            node.firstChild.remove();
        }
    }

    const SPACE = document.createTextNode(' ');
    /**
     * Draw dice for the given player
     * @param {number} pid index of player 
     * @param {HTMLElement} cell column to add the dice images
     */
    function drawDice(pid, cell) {
        $(cell).addClass('hidden');
        removeChildren(cell);
        for (let dice of myGame.players[pid].hand) {
            cell.appendChild(images[dice].cloneNode(false));
            cell.appendChild(SPACE.cloneNode(false));
        }
        const adj = myGame.players[pid].adjustment;
        if (adj) {
            cell.innerHTML += '<br><span class="label label-default">' + adj + '</span>';
        }
        $(cell).removeClass('hidden');
    }

    function drawBid(bid, trow) {
        const bidTxt = JSON.stringify(bid);
        if (bidTxt != my.bidTxt) {
            my.bidTxt = bidTxt;
            if (bid.n > 0 && bid.face > 0) {
                const cell = document.createElement('td');
                cell.appendChild(document.createTextNode(bid.n + ' '));
                cell.appendChild(images[bid.face].cloneNode(false));
                trow.replaceChild(cell, trow.cells[2]);
            }
        }
    }

    function pollStatus() {
        clearTimeout(my.timer);
        my.timer = setTimeout(function(){ getStatus(); }, 5000);
    }

    function handleStateWaiting() {
        $('#welcomeMessage').html(my.name + ', waiting for others' +
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

        // This is an active page, so needs to be updated
        pollStatus();
    }

    const PLAY_ICON = document.createElement('span');
    PLAY_ICON.setAttribute('class', 'glyphicon glyphicon-play');

    const WINNER_ICON = document.createElement('span');
    WINNER_ICON.setAttribute('class', 'glyphicon glyphicon-thumbs-up');

    const LOSER_ICON = document.createElement('span');
    LOSER_ICON.setAttribute('class', 'glyphicon glyphicon-thumbs-down');

    /**
     * Draw a given row of the table showing dice and bids.
     * @param trow [in] row to draw
     * @param pid [in] index of player to draw
     */
    function drawTableRow(trow, pid) {
        let name = splitString(myGame.players[pid].name, 10);
        const cell = document.createElement('td');
        if (myGame.players[pid].winner) {
            $(trow).removeClass('active');
            cell.appendChild(WINNER_ICON.cloneNode(false));
            cell.appendChild(SPACE.cloneNode(false));
        } else if (myGame.players[pid].loser) {
            $(trow).removeClass('active');
            cell.appendChild(LOSER_ICON.cloneNode(false));
            cell.appendChild(SPACE.cloneNode(false));
        } else if (pid == myGame.turn) {
            $(trow).addClass('active');
            cell.appendChild(PLAY_ICON.cloneNode(false));
            cell.appendChild(SPACE.cloneNode(false));
        } else {
            $(trow).removeClass('active');
        }
        if (name == my.name) {
            const nameEl = document.createElement('strong');
            nameEl.appendChild(document.createTextNode(name));
            cell.appendChild(nameEl);
        } else {
            cell.appendChild(document.createTextNode(name));
        }

        trow.replaceChild(cell, trow.cells[0]);
        drawDice(pid, trow.cells[1]);
        drawBid(myGame.players[pid].bid, trow);
    }

    /**
     * Check if number the dice should be redrawn and save current dice as baseline
     */
    function checkDiceChanged() {
        let allDice = [];
        for (const player of myGame.players) {
            for (const dice of player.hand) {
                allDice.push(dice);
            }
        }
        allDice = allDice.toString();
        if (allDice == my.allDice) {
            console.log('no change in dice');
            return false;
        }
        console.log('dice changed');
        console.log(allDice.toString());
        my.allDice = allDice;
        return true;
    }

    const TD = document.createElement('td');
    const TR = document.createElement('tr');
    TR.appendChild(TD.cloneNode(false));
    TR.appendChild(TD.cloneNode(false));
    TR.appendChild(TD.cloneNode(false));
    /**
     * Make sure table has correct number of rows.
     * @param {HTMLElement} table
     * @param {int} numPlayers
     */
    function addRowsToTable(table, numPlayers) {
        while ((table.rows.length - 1) < numPlayers) {
            table.appendChild(TR.cloneNode(true));
        }
        while ((table.rows.length - 1) > numPlayers) {
            table.removeChild(table.lastChild);
        }
    }

    /** Draw the table that shows players, their dice and bids */
    function drawTable() {
        const table = document.getElementById('player-table');
        if (checkDiceChanged()) {
            $(table).addClass('hidden');
        }
        const numPlayers = myGame.players.length;
        addRowsToTable(table, numPlayers);
        for (let i = 0; i < numPlayers; ++i) {
            drawTableRow(table.rows[i + 1], i);
        }
        $(table).removeClass('hidden');
    }

    /** @returns winners, losers and how many dice lost by losers */
    function getWinnersAndLosers() {
        let winners = [];
        let losers = [];
        let diceLost = 0;
        for (const player of myGame.players) {
            if (player.winner) {
                winners.push(player.name);
            } else if (player.loser) {
                losers.push(player.name);
                diceLost = -player.adjustment;
            }
        }
        return {winners: winners, losers: losers, diceLost: diceLost};
    }

    function handleRoundStarted() {
        const playerInTurn = myGame.players[myGame.turn].name;
        if (myGame.players[myGame.turn].name == my.name) {
            $('#game-msg').html(my.name + ', it\'s your turn. You can either make a higher bid or challeng the bid.');
            if (myGame.bid.n) {
                $('#challenge').removeClass('disabled');
            }
            else {
                $('#challenge').addClass('disabled');
            }
            $('#BidOrChallenge').removeClass('hidden');
            my.bid.n = myGame.bid.n;
            my.bid.face = myGame.bid.face ? myGame.bid.face : 1;
            drawMyBid();
            // No need to poll status when it's my turn
        }
        else {
            $('#game-msg').html('Waiting for ' + playerInTurn + ' to bid or challenge.');
            $('#BidOrChallenge').addClass('hidden');
            pollStatus();
        }
        $('#DoneViewingResults').addClass('hidden');    
    }

    function handleRoundFinished() {
        const wl = getWinnersAndLosers();
        $('#game-msg').html('Round ended. ' + wl.winners.join(', ') + ' won. ' +
                            wl.losers.join(', ') + ' lost ' + wl.diceLost + ' dice.<br>' +
                            'Click done to start new round.');
        $('#BidOrChallenge').addClass('hidden');
        $('#DoneViewingResults').removeClass('hidden');
        // Not polling status here. Player will start the round when he or she is ready.
    }

    function handleGameFinished() {
        const wl = getWinnersAndLosers();
        $('#game-msg').html('Game ended. Winner is <strong>' + wl.winners.join(', ') + '</strong>.<br>' +
                            'Click done to start new round.');
        $('#BidOrChallenge').addClass('hidden');
        $('#DoneViewingResults').removeClass('hidden');
        // Not polling status here. Player will start the round when he or she is ready.
    }

    function handleGameState() {
        var t0 = performance.now();
        drawTable();
        var t1 = performance.now();
        console.log('drawTable() took ' + (t1 - t0) + ' milliseconds.');
        if (myGame.state == 'ROUND_STARTED') {
            handleRoundStarted();
        } else if (myGame.state == 'CHALLENGE') {
            handleRoundFinished();
        } else if (myGame.state == 'GAME_FINISHED') {
            handleGameFinished();
        }
        show('GameOn');
    }
    function join(game) {
        $('#join-status').addClass('hidden');
        $('#create-status').addClass('hidden');
        $.post('/api/join', JSON.stringify({id: my.id, game: game}), function(json) {
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
        $.post('/api/newGame', JSON.stringify({id: my.id, game: name}), function(json) {
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
        $.post('/api/startGame', JSON.stringify({id: my.id}), function(json) {
            console.log(json);
            if (json.success || json.error == 'GAME_ALREADY_STARTED') {
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
        $.post('/api/startRound', JSON.stringify({id: my.id}), function(json) {
            console.log(json);
            getStatus();
        }, 'json');
    }

    function bid() {
        console.log('bid');
        $.post('/api/bid', JSON.stringify({id: my.id, n: my.bid.n, face: my.bid.face}), function(json) {
            console.log(json);
            if (json.success) getStatus();
        }, 'json');
    }

    function challenge() {
        console.log('challenge');
        $.post('/api/challenge', JSON.stringify({id: my.id}), function(json) {
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

    function logOut() {
        console.log('logging out');
        $.post('/api/logout', JSON.stringify({id: my.id}), function(json) {
            console.log(json);
            getStatus();
        }, 'json');
    }

    function selectedGame() {
        const selectedGame = $('#games').val();
        for (let game of my.availableGames) {
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
            my.availableGames = json;
            let gameSelect = $('#games');
            gameSelect.empty();
            for (let game of json) {
                gameSelect.append(new Option(game.game, game.game));
            }
            selectedGame();
        }, 'json');
    }

    function handleJoinCreate() {
        $('#welcomeMessage').html('Welcome, ' + my.name + '.' +
                                  ' You can select an existing game below and join' +
                                  ' it. You can click the refresh' +
                                  ' button on the right side of games to see newly' +
                                  ' created games.');
        refreshGames();
        show(['welcomeMessage', 'SetupCreate']);
    }

    function getStatus() {
        console.log('getStatus');
        $.post('/api/status', JSON.stringify({id: my.id, hash: my.hash}), function (json) {
            if (json.success) {
                if (json.noChange) {
                    return pollStatus();
                }
                console.log(json);
                if (json.game !== undefined) {
                    $('#LogOut').removeClass('hidden');
                    my.hash = json.game.hash;
                    my.name = json.name;
                    myGame = json.game;
                    if (myGame.state != 'GAME_NOT_STARTED') {
                        handleGameState();
                    } else if (myGame.state == 'GAME_NOT_STARTED') {
                        handleStateWaiting();
                    }
                } else {
                    $('#LogOut').addClass('hidden');
                    my.name = json.name;
                    handleJoinCreate();
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
        my.bid.n += offset;
        drawMyBid();
    }

    function adjustFace(offset) {
        const face = my.bid.face + offset;
        my.bid.face = face < 1 ? 6 : face > 6 ? 1 : face; 

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
        $('#log-out').click(function() {
            logOut();
        });
        my.id = getParameterByName('id');
        console.log('Loaded content for ' + my.id);
        getStatus();
    });
})();

