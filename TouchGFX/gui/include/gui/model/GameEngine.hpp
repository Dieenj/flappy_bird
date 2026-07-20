#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP
#include <stdint.h>

// ── Màn hình landscape 320x240 ──
static const int SCREEN_W   = 320;
static const int SCREEN_H   = 240;
static const int GROUND_H   = 20;
static const int GROUND_TOP = 220;  // SCREEN_H - GROUND_H
static const int PIPE_W     = 35;
static const int PIPE_CAP_W = 43;
static const int PIPE_CAP_H = 10;
static const int BIRD_X     = 70;   // X cố định của chim
static const int BIRD_R     = 12;   // collision radius
static const int MAX_PIPES  = 5;

// ── Physics cố định toàn game ──
static constexpr float FIXED_GRAVITY  = 0.20f;
static constexpr float FIXED_JUMP     = 4.0f;
static constexpr float FIXED_TERM_VEL = 12.0f;

// ── Speed tăng theo sqrt(score), lerp mượt ──
static constexpr float SPEED_BASE   = 70.0f  / 60.0f;
static constexpr float SPEED_SQRT_K = 15.0f  / 60.0f;
static constexpr float SPEED_MAX    = 210.0f / 60.0f;
static constexpr float SPEED_LERP   = 2.5f   / 60.0f;

// ── Bảng cấp: chỉ gap và khoảng cách spawn thay đổi ──
struct Level {
    const char* name;
    uint16_t    scoreThreshold;
    int         gapSize;
    int         pipeDist;
};

static const Level LEVELS[] = {
    { "Easy",     0, 110, 200 },
    { "Normal",   5, 100, 190 },
    { "Medium",  12,  90, 180 },
    { "Hard",    20,  80, 170 },
    { "Intense", 30,  72, 165 },
    { "HELL",    45,  65, 160 },
};
static const int NUM_LEVELS = 6;

enum PipeType { PIPE_NORMAL = 0, PIPE_MOVING = 1 };

struct Pipe {
    float    x;
    float    gapY;
    float    origGapY;
    float    phase;
    int      gap;       // snapshot khi spawn - tránh nháy khi lên cấp
    PipeType type;
    bool     passed;
};

// ──────────────────────────────────────────────────────────────
class GameEngine
{
public:
    enum State { WELCOME, PLAYING, PAUSED, COUNTDOWN, DEAD };

    void init(uint32_t seed) {
        _randSeed = seed ^ 0xDEADBEEF;
        _best     = 0;
        _state    = WELCOME;
        resetGame();
    }

    // Gọi mỗi 16.6ms từ GamePresenter::handleTickEvent()
    void tick() {
        if (_state == COUNTDOWN) {
            if (++_cdTick >= 60) {
                _cdTick = 0;
                _cdNum--;
                _countdownEvent = true;
                if (_cdNum == 0) _state = PLAYING;
            }
            return;
        }
        if (_state != PLAYING) return;
        _frame++;

        // Lerp speed mượt theo score
        float tgt = SPEED_BASE + sqrtA((float)_score) * SPEED_SQRT_K;
        if (tgt > SPEED_MAX) tgt = SPEED_MAX;
        _dispSpeed += (tgt - _dispSpeed) * SPEED_LERP;

        // Physics chim - gravity & jump cố định
        _birdVy += FIXED_GRAVITY;
        if (_birdVy > FIXED_TERM_VEL) _birdVy = FIXED_TERM_VEL;
        _birdY += _birdVy;

        // Di chuyển và update ống
        for (int i = 0; i < _pipeCount; i++) {
            Pipe& pp = _pipes[i];
            pp.x -= _dispSpeed;

            if (pp.type == PIPE_MOVING) {
                pp.phase += 0.04f;
                if (pp.phase > 6.283f) pp.phase -= 6.283f;
                float minGY = (float)(PIPE_CAP_H + 2);
                float maxGY = (float)(GROUND_TOP - pp.gap - PIPE_CAP_H - 2);
                pp.gapY = pp.origGapY + sinA(pp.phase) * 24.0f;
                if (pp.gapY < minGY) pp.gapY = minGY;
                if (pp.gapY > maxGY) pp.gapY = maxGY;
            }

            // Ghi điểm khi qua ống
            if (!pp.passed && pp.x + PIPE_W < (float)BIRD_X) {
                pp.passed = true;
                _score++;
                _scoreEvent = true;
                if (_score > _best) _best = _score;
                int nl = levelIdx(_score);
                if (nl != _levelIdx) {
                    _levelIdx    = nl;
                    _levelUpEvent = true;
                }
            }
        }

        // Xóa ống ra khỏi màn trái
        int k = 0;
        for (int i = 0; i < _pipeCount; i++)
            if (_pipes[i].x + PIPE_CAP_W > 0) _pipes[k++] = _pipes[i];
        _pipeCount = k;

        // Spawn ống mới
        if (_pipeCount == 0) {
            if (++_graceTimer > 90) { _graceTimer = 0; spawnPipe(true); }
        } else {
            float lastX = _pipes[_pipeCount - 1].x;
            if ((float)SCREEN_W - lastX >= (float)_nextDist) spawnPipe(false);
        }

        // Collision detection
        float r = BIRD_R * 0.8f;
        if (_birdY - r < 0 || _birdY + r > (float)GROUND_TOP) { die(); return; }
        for (int i = 0; i < _pipeCount; i++) {
            const Pipe& pp = _pipes[i];
            float dx = (float)BIRD_X - (pp.x + PIPE_W / 2.0f);
            if (dx < 0) dx = -dx;
            if (dx < r + PIPE_W / 2.0f + 2.0f) {
                if (_birdY - r < pp.gapY || _birdY + r > pp.gapY + pp.gap)
                    { die(); return; }
            }
        }
    }

    void tickDead() { if (_state == DEAD) _deathTimer++; }

    // Tap màn hình
    void flap() {
        if (_state == COUNTDOWN) return;
        if (_state == WELCOME || _state == DEAD) {
            _state = PLAYING;
            resetGame();
            return;
        }
        if (_state == PLAYING) _birdVy = -FIXED_JUMP;
    }

    // PA0 button
    void togglePause() {
        if (_state == PLAYING) {
            _state = PAUSED;
        } else if (_state == PAUSED) {
            _state = COUNTDOWN;
            _cdNum = 3; _cdTick = 0;
            _countdownEvent = true;
        }
    }

    void goHome() { _state = WELCOME; }

    // ── Getters ──
    State       getState()        const { return _state; }
    float       getBirdY()        const { return _birdY; }
    int         getPipeCount()    const { return _pipeCount; }
    const Pipe& getPipe(int i)    const { return _pipes[i]; }
    uint32_t    getScore()        const { return _score; }
    uint32_t    getBest()         const { return _best; }
    int         getLevelIdx()     const { return _levelIdx; }
    const char* getLevelName()    const { return LEVELS[_levelIdx].name; }
    int         getCdNum()        const { return _cdNum; }
    uint32_t    getDeathTimer()   const { return _deathTimer; }

    // ── Events (consume-once) ──
    bool consumeScoreEvent()   { bool v=_scoreEvent;    _scoreEvent=false;    return v; }
    bool consumeLevelUpEvent() { bool v=_levelUpEvent;  _levelUpEvent=false;  return v; }
    bool consumeHitEvent()     { bool v=_hitEvent;      _hitEvent=false;      return v; }
    bool consumeCdEvent()      { bool v=_countdownEvent;_countdownEvent=false;return v; }

    static int levelIdx(uint32_t s) {
        int i = 0;
        for (int j = 1; j < NUM_LEVELS; j++)
            if (s >= (uint32_t)LEVELS[j].scoreThreshold) i = j;
        return i;
    }

private:
    State    _state      = WELCOME;
    float    _birdY      = (float)SCREEN_H / 2;
    float    _birdVy     = 0.0f;
    float    _dispSpeed  = SPEED_BASE;
    uint32_t _score      = 0;
    uint32_t _best       = 0;
    uint32_t _frame      = 0;
    int      _levelIdx   = 0;
    int      _nextDist   = 200;
    int      _lastGapY   = SCREEN_H / 2 - 30;
    uint32_t _graceTimer = 0;
    uint32_t _deathTimer = 0;
    int      _cdNum      = 3;
    uint32_t _cdTick     = 0;
    Pipe     _pipes[MAX_PIPES];
    int      _pipeCount  = 0;
    bool     _scoreEvent     = false;
    bool     _levelUpEvent   = false;
    bool     _hitEvent       = false;
    bool     _countdownEvent = false;
    uint32_t _randSeed       = 12345;

    uint32_t myRand() {
        _randSeed ^= _randSeed << 13;
        _randSeed ^= _randSeed >> 17;
        _randSeed ^= _randSeed << 5;
        return _randSeed;
    }

    static float sqrtA(float x) {
        if (x <= 0.0f) return 0.0f;
        float r = x * 0.5f, y = x;
        for (int i = 0; i < 3; i++) y = y * 0.5f + r / y;
        return y;
    }

    static float sinA(float x) {
        while (x >  3.14159f) x -= 6.28318f;
        while (x < -3.14159f) x += 6.28318f;
        float x2 = x * x;
        return x * (1.0f - x2 * (0.16667f - x2 * 0.00833f));
    }

    void resetGame() {
        _birdY      = (float)SCREEN_H / 2.0f;
        _birdVy     = -FIXED_JUMP;
        _dispSpeed  = SPEED_BASE;
        _pipeCount  = 0;
        _score      = 0;
        _frame      = 0;
        _levelIdx   = 0;
        _nextDist   = LEVELS[0].pipeDist;
        _lastGapY   = SCREEN_H / 2 - 30;
        _graceTimer = 0;
        _deathTimer = 0;
    }

    void spawnPipe(bool first) {
        if (_pipeCount >= MAX_PIPES) return;
        int li  = levelIdx(_score);
        int gap = LEVELS[li].gapSize;
        int minY = PIPE_CAP_H + 2;
        int maxY = GROUND_TOP - gap - PIPE_CAP_H - 2;
        if (maxY <= minY) maxY = minY + GROUND_H;

        int gapY;
        if (first) {
            gapY = (minY + maxY) / 2;
        } else {
            int maxDelta = 40 + li * 8;
            int delta    = (int)(myRand() % (uint32_t)(maxDelta * 2 + 1)) - maxDelta;
            gapY = _lastGapY + delta;
            if (gapY < minY) gapY = minY;
            if (gapY > maxY) gapY = maxY;
        }
        _lastGapY = gapY;

        bool moving = (_score >= 12) && ((myRand() % 100) < 35);
        Pipe& pp    = _pipes[_pipeCount++];
        pp.x        = (float)(SCREEN_W + PIPE_W);
        pp.gapY     = (float)gapY;
        pp.origGapY = (float)gapY;
        pp.gap      = gap;
        pp.type     = moving ? PIPE_MOVING : PIPE_NORMAL;
        pp.passed   = false;
        pp.phase    = (float)(myRand() % 628) / 100.0f;
        _nextDist   = LEVELS[levelIdx(_score)].pipeDist;
    }

    void die() { _state = DEAD; _deathTimer = 0; _hitEvent = true; }
};

#endif // GAME_ENGINE_HPP
