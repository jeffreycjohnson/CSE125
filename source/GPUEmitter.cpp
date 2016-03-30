#include "GPUEmitter.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Timer.h"

using namespace std;

GPUEmitter::GPUEmitter(GameObject* go, string tex, bool burstEmitter)
{
	gameObject = go;
	texture = new Texture(tex);

	prevPosition = gameObject->transform.getWorldPosition();
	velocity = { 0, 0, 0 };
	minStartSize = 0.4;
	maxStartSize = 0.6;
	minEndSize = 0.6;
	maxEndSize = 1;
	startOpacity = 1;
	endOpacity = 0;
	minDuration = 0;
	maxDuration = 3;
	minStartColor = { 1, 1, 1 };
	maxStartColor = { 1, 1, 1 };
	minEndColor = { 1, 1, 1 };
	maxEndColor = { 1, 1, 1 };
	minStartVelocity = { -5, -10, -5 };
	maxStartVelocity = { 10, 5, 10 };
	minAcceleration = { -30, 0, -30 };
	maxAcceleration = { 0, 30, 0 };
	minStartAngle = 0;
	maxStartAngle = 360;
	minAngularVelocity = -10;
	maxAngularVelocity = 10;
	emitterVelocity = { 0, 0, 0 };
	emitterVelocityScale = 2;
	burst = burstEmitter;
	trigger = false;
	count = 4000;
	enabled = false;
	loop = false;
	additive = true;
	rotateTowardsVelocity = true;
	startTime = Timer::time();
}

GPUEmitter::~GPUEmitter()
{
	//delete texture;
	// Delete for arrays is handled in genParticles
}

void GPUEmitter::update(float deltaTime)
{
	if (burst)
	{
		if (loop && (Timer::time() - startTime) > maxDuration)
			play();

		if (trigger)
		{
			burstSeed = rand();
			burstStartPos = { gameObject->transform.getWorldPosition().x,
				gameObject->transform.getWorldPosition().y,
				gameObject->transform.getWorldPosition().z };
			trigger = false;
		}
	}

	prevPosition = gameObject->transform.getWorldPosition();
}

void GPUEmitter::draw()
{
    if (generateParticles) {
        genParticles();
        generateParticles = false;
    }
	if (enabled)
	{
		if (burst)
			Renderer::switchShader(EMITTER_BURST_SHADER);
		else
			Renderer::switchShader(EMITTER_SHADER);

        Shader& shader = Renderer::getCurrentShader();

        shader["elapsedTime"] = (GLfloat)Timer::time();
        setUniforms();

		glEnable(GL_BLEND);
		if (additive)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(false);
		texture->bindTexture(0);

		Renderer::setModelMatrix(glm::mat4(1));
		if (Renderer::gpuData.vaoHandle != vao) {
			glBindVertexArray(vao);
			Renderer::gpuData.vaoHandle = vao;
		}
		glDrawArrays(GL_QUADS, 0, count * 4);
		glBindVertexArray(NULL);
		glDepthMask(true);
		glDisable(GL_BLEND);
	}
}

void GPUEmitter::init()
{
    generateParticles = true;
	if (!burst)
		enabled = true;
}

void GPUEmitter::play()
{
	trigger = true;
	enabled = true;
	startTime = Timer::time();
}

GLuint GPUEmitter::genParticles()
{
	if (burst)
		Renderer::switchShader(EMITTER_BURST_SHADER);
	else
	{
		Renderer::switchShader(EMITTER_SHADER);
		startTimes = new float[count * 4];
	}

	durations = new float[count * 4];
	quadCorners = new int[count * 4];
	seeds = new unsigned int[count * 4];

	srand(Timer::time());

	for (int i = 0; i < count * 4 - 3; i += 4)
	{
		if (!burst)
			startTimes[i] = startTimes[i + 1] = startTimes[i + 2] = startTimes[i + 3] = (float) i / count + Timer::time();

		seeds[i] = seeds[i + 1] = seeds[i + 2] = seeds[i + 3] = rand();
		quadCorners[i] = i % 4;
		quadCorners[i + 1] = (i + 1) % 4;
		quadCorners[i + 2] = (i + 2) % 4;
		quadCorners[i + 3] = (i + 3) % 4;

		float rnd = minDuration + (((maxDuration - minDuration) * rand()) / (RAND_MAX + 1.0f));
		durations[i] = durations[i + 1] = durations[i + 2] = durations[i + 3] = rnd;
	}

	GLuint duration_vbo;
	glGenBuffers(1, &duration_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, duration_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(durations[0]) * count * 4, durations, GL_STATIC_DRAW);

	GLuint corner_vbo;
	glGenBuffers(1, &corner_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, corner_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadCorners[0]) * count * 4, quadCorners, GL_STATIC_DRAW);

	GLuint seed_vbo;
	glGenBuffers(1, &seed_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, seed_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(seeds[0]) * count * 4, seeds, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	if (!burst)
	{
		GLuint time_vbo;
		glGenBuffers(1, &time_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, time_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(startTimes[0]) * count * 4, startTimes, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, time_vbo);
		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, duration_vbo);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, corner_vbo);
	glVertexAttribIPointer(2, 1, GL_INT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, seed_vbo);
	glVertexAttribIPointer(3, 1, GL_INT, 0, NULL);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glBindVertexArray(NULL);

    setUniforms();

	// We don't need the arrays anymore, since we already passed them to the GPU
	if (!burst)
		delete[] startTimes;

	delete[] durations;
	delete[] quadCorners;
	delete[] seeds;

	return vao;
}

void GPUEmitter::setUniforms()
{
    Shader& shader = Renderer::getCurrentShader();

    if (burst)
    {
        shader["startTime"] = (GLfloat)startTime;
        shader["burstSeed"] = (GLuint)burstSeed;
        shader["emitterPos"] = burstStartPos;
        shader["emitterVelocity"] = emitterVelocity * emitterVelocityScale;
    }
    else
    {
        shader["emitterPos"] = gameObject->transform.getWorldPosition();
        shader["emitterVelocity"] = (gameObject->transform.getWorldPosition() - prevPosition) * emitterVelocityScale;
    }
    shader["minVelocity"] = minStartVelocity;
    shader["maxVelocity"] = maxStartVelocity;
    shader["minAcceleration"] = minAcceleration;
    shader["maxAcceleration"] = maxAcceleration;
    shader["minStartSize"] = minStartSize;
    shader["maxStartSize"] = maxStartSize;
    shader["minEndSize"] = minEndSize;
    shader["maxEndSize"] = maxEndSize;
    shader["minStartColor"] = minStartColor;
    shader["maxStartColor"] = maxStartColor;
    shader["minEndColor"] = minEndColor;
    shader["maxEndColor"] = maxEndColor;
    shader["startOpacity"] = startOpacity;
    shader["endOpacity"] = endOpacity;
    shader["minStartAngle"] = minStartAngle;
    shader["maxStartAngle"] = maxStartAngle;
    shader["minAngularVelocity"] = minAngularVelocity;
    shader["maxAngularVelocity"] = maxAngularVelocity;
    shader["rotateTowardsVelocity"] = (unsigned int)(rotateTowardsVelocity ? 1 : 0);
}
