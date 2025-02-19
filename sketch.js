const TILE_SIZE = 64;
const MAP_NUM_ROWS = 11;
const MAP_NUM_COLS = 15;
const WINDOW_WIDTH = MAP_NUM_COLS * TILE_SIZE;
const WINDOW_HEIGHT = MAP_NUM_ROWS * TILE_SIZE;
const FOV_ANGLE = 60 * (Math.PI / 180);
const WALL_STRIP_WIDTH = 25;
const NUM_RAYS = WINDOW_WIDTH / WALL_STRIP_WIDTH;

class Map {
  constructor() {
    this.grid = [
      [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
      [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],
      [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1],
      [1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
      [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
      [1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1],
      [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
      [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
      [1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1],
      [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
      [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
    ];
  }
  render() {
    for (var i = 0; i < MAP_NUM_ROWS; i++) {
      for (var j = 0; j < MAP_NUM_COLS; j++) {
        var tileX = j * TILE_SIZE;
        var tileY = i * TILE_SIZE;
        var tileColor = this.grid[i][j] == 1 ? "#222" : "#fff";
        stroke("#222");
        fill(tileColor);
        rect(tileX, tileY, TILE_SIZE, TILE_SIZE);
      }
    }
  }
  hasWallAt(x, y) {
    var mapGridIndexX = Math.floor(x / TILE_SIZE);
    var mapGridIndexY = Math.floor(y / TILE_SIZE);

    return this.grid[mapGridIndexY][mapGridIndexX] == 1;
  }
}

class Player {
  constructor() {
    this.x = WINDOW_WIDTH / 2;
    this.y = WINDOW_HEIGHT / 2;
    this.radius = 3;
    this.turnDirection = 0; // -1 if left, +1 if right
    this.walkDirection = 0; // -1 if back, +1 if front
    this.rotationAngle = Math.PI / 2;
    this.moveSpeed = 2.0; // 2px / frame
    this.rotationSpeed = 2 * (Math.PI / 180);
  }
  render() {
    noStroke();
    fill("red");
    circle(this.x, this.y, this.radius);
  }
  update() {
    this.rotationAngle += this.turnDirection * this.rotationSpeed;
    var moveStep = this.walkDirection * this.moveSpeed;

    var newXPosition = this.x + moveStep * Math.cos(this.rotationAngle);
    var newYPosition = this.y + moveStep * Math.sin(this.rotationAngle);

    if (!grid.hasWallAt(newXPosition, newYPosition)) {
      this.x = newXPosition;
      this.y = newYPosition;
    }
  }
}

class Ray {
  constructor(rayAngle) {
    this.rayAngle = normalizeAngle(rayAngle);
    this.wallHitX = 0;
    this.wallHitY = 0;
    this.distance = 0;
    this.wasHitVertical = false;
    this.isRayFacingDown = this.rayAngle > 0 && this.rayAngle < Math.PI;
    this.isRayFacingUp = !this.isRayFacingDown;
    this.isRayFacingRight = this.rayAngle < (Math.PI * 0.5) || this.rayAngle > 1.5 * Math.PI;
    this.isRayFacingLeft = !this.isRayFacingRight;
  }
  render() {
    stroke("rgba(255, 0, 0, 0.3)");
    line(
      player.x,
      player.y,
      this.wallHitX,
      this.wallHitY
    );
  }
  cast(columnIndex) {
    var xintercept, yintercept;
    var xstep, ystep;

    // Compute horizontal intersection with the grid
    var foundHorizontalWallHit = false;
    var horizontalWallHitX = 0;
    var horizontalWallHitY = 0;

    yintercept = Math.floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += this.isRayFacingDown ? TILE_SIZE : 0;
    xintercept = player.x + ((yintercept - player.y) / Math.tan(this.rayAngle));

    // Compute steps
    ystep = TILE_SIZE;
    ystep *= this.isRayFacingUp ? -1 : 1;
    xstep = TILE_SIZE / Math.tan(this.rayAngle);
    xstep *= this.isRayFacingLeft && xstep > 0 ? -1 : 1;
    xstep *= this.isRayFacingRight && xstep < 0 ? -1 : 1;

    var nextHorizontalIntersectionX = xintercept;
    var nextHorizontalIntersectionY = yintercept;

    while (nextHorizontalIntersectionX >= 0 && nextHorizontalIntersectionX <= WINDOW_WIDTH && nextHorizontalIntersectionY >= 0 && nextHorizontalIntersectionY <= WINDOW_HEIGHT) {
      if (grid.hasWallAt(nextHorizontalIntersectionX, nextHorizontalIntersectionY - (this.isRayFacingUp ? 1 : 0))) {
        foundHorizontalWallHit = true;
        horizontalWallHitX = nextHorizontalIntersectionX;
        horizontalWallHitY = nextHorizontalIntersectionY;
        break;
      } else {
        nextHorizontalIntersectionX += xstep;
        nextHorizontalIntersectionY += ystep;
      }
    }

    // Compute vertical intersection with the grid
    var foundVerticalWallHit = false;
    var verticalWallHitX = 0;
    var verticalWallHitY = 0;

    xintercept = Math.floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += this.isRayFacingRight ? TILE_SIZE : 0;
    yintercept = player.y + (xintercept - player.x) * Math.tan(this.rayAngle);

    // Compute steps
    xstep = TILE_SIZE;
    xstep *= this.isRayFacingLeft ? -1 : 1;
    ystep = TILE_SIZE * Math.tan(this.rayAngle);
    ystep *= this.isRayFacingUp && ystep > 0 ? -1 : 1;
    ystep *= this.isRayFacingDown && ystep < 0 ? -1 : 1;

    var nextVerticalIntersectionX = xintercept;
    var nextVerticalIntersectionY = yintercept;

    while (nextVerticalIntersectionX >= 0 && nextVerticalIntersectionX <= WINDOW_WIDTH && nextVerticalIntersectionY >= 0 && nextVerticalIntersectionY <= WINDOW_HEIGHT) {
      
      if (grid.hasWallAt(nextVerticalIntersectionX - (this.isRayFacingLeft ? 1 : 0), nextVerticalIntersectionY)) {
        foundVerticalWallHit = true;
        verticalWallHitX = nextVerticalIntersectionX;
        verticalWallHitY = nextVerticalIntersectionY;
        break;
      } else {
        nextVerticalIntersectionX += xstep;
        nextVerticalIntersectionY += ystep;
      }
    }

    // Compute shortest distance between vertical intersection point and player and horizontal intersection point and player
    var horizontalIntersectionDistance = foundHorizontalWallHit ? distanceBetweenTwoPoints(player.x, player.y, horizontalWallHitX, horizontalWallHitY) : Number.MAX_VALUE;
    var verticalIntersectionDistance = foundVerticalWallHit ? distanceBetweenTwoPoints(player.x, player.y, verticalWallHitX, verticalWallHitY) : Number.MAX_VALUE;

    this.wallHitX = horizontalIntersectionDistance < verticalIntersectionDistance ? horizontalWallHitX : verticalWallHitX;
    this.wallHitY = horizontalIntersectionDistance < verticalIntersectionDistance ? horizontalWallHitY : verticalWallHitY;
    this.distance = horizontalIntersectionDistance < verticalIntersectionDistance ? horizontalIntersectionDistance : verticalIntersectionDistance;
    this.wasHitVertical = verticalIntersectionDistance < horizontalIntersectionDistance;
  }
}

var player = new Player();
var grid = new Map();
var rays = [];

function normalizeAngle(angle) {
  angle = angle % (2 * Math.PI);
  if (angle < 0) {
    angle += (2 * Math.PI)
  }
  return angle;
}

function castAllRays() {
  var currentColumnIndex = 0;
  var rayAngle = player.rotationAngle - (FOV_ANGLE / 2);
  rays = [];
  for (var i = 0; i < NUM_RAYS; i++) {
    var ray = new Ray(rayAngle);
    ray.cast(currentColumnIndex);
    rays.push(ray);

    rayAngle += FOV_ANGLE / NUM_RAYS;
    currentColumnIndex++;
  }
}

function distanceBetweenTwoPoints(x1, y1, x2, y2) {
  return Math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

function keyPressed() {
  if (keyCode == UP_ARROW) {
    player.walkDirection = +1;
  } else if (keyCode == DOWN_ARROW) {
    player.walkDirection = -1;
  } else if (keyCode == RIGHT_ARROW) {
    player.turnDirection = +1;
  } else if (keyCode == LEFT_ARROW) {
    player.turnDirection = -1;
  }
}

function keyReleased() {
  if (keyCode == UP_ARROW) {
    player.walkDirection = 0;
  } else if (keyCode == DOWN_ARROW) {
    player.walkDirection = 0;
  } else if (keyCode == RIGHT_ARROW) {
    player.turnDirection = 0;
  } else if (keyCode == LEFT_ARROW) {
    player.turnDirection = 0;
  }
}

function setup() {
  createCanvas(WINDOW_WIDTH, WINDOW_HEIGHT);
}

function update() {
  player.update();
  castAllRays();
}

function draw() {
  update();
  
  grid.render();
  for (ray of rays) {
    ray.render();
  }
  player.render();
}
