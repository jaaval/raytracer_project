#include "Renderer.h"
#include <vector>
#include <iostream>
#include <chrono> 
#include <random>
#include "ThreadPool.h"
#include "utilmath.h"



std::vector<std::vector<int>> createTiles(int w, int h, int nthr) 
{
	std::vector<std::vector<int>> tiles;

	int dh, dw;
	if (nthr > 3)
	{
		int numtiles = sqrt(nthr * nthr / 2);
		numtiles = std::max(numtiles, 1);
		dh = h / numtiles;
		dw = w / numtiles;
	}
	else {
		dh = h;
		dw = w / nthr;
	}
	
	dh = std::max(dh, 24); // minimum tile size, not much sense to do smaller than 12x12
	dw = std::max(dw, 24);
	int sw, sh, ew, eh;
	
	for (int i = 0; i < w; i+=dw)
	{
		for (int j = 0; j < h; j+=dh)
		{
			sh = j; sw = i;
			eh = std::min(h, j + dh);
			ew = std::min(w, i + dw);

			std::vector<int> tile = {sw, sh, ew, eh};
			tiles.push_back(tile);
		}
	}
	// suffle the tiles just for fun
	std::random_device rd;
	auto rng = std::default_random_engine{ rd() };
	std::shuffle(std::begin(tiles), std::end(tiles), rng);
	return tiles;
}


void tileTask(int id, ImageData& imdata, const Scene& scene, const std::vector<int>& tile)
{
	auto start = std::chrono::system_clock::now();
	int height, width;
	std::tie(width, height) = scene.getScreenSize();
	int index;
	Color color;
	int tilesize = (tile[2] - tile[0]) * (tile[3] - tile[1]);
	const Vec3d campos = scene.getCamPos();
	
	for (int j = tile[1]; j < tile[3]; j++)
	{
		for (int i = tile[0]; i < tile[2]; i++)
		{
			
			index = j * width + i;
			const Vec3d dir = scene.getPixelLocation(index);
			Ray a(campos, 1);
			a.setDirection2point(dir);
			color = a.march(scene);
			imdata.storeValues(index, color.colors());
		}
	}
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start);

	std::cout << "id: " << id << " tile: " <<
		tile[0] << " " << tile[1] << " " << tile[2] << " " << tile[3] <<
		" done in " << elapsed.count() << "seconds" <<std::endl;
}



void renderImage(const Scene& scene, const int nThreads)
{
	auto start = std::chrono::system_clock::now();
	std::pair<int, int> screenSize = scene.getScreenSize();
	TP::ThreadPool pool(nThreads);
	std::vector<std::vector<int>> tiles = createTiles(screenSize.first, screenSize.second, nThreads);

	ImageData imagedata(screenSize.first, screenSize.second);
	MinWindow window;
	window.init(screenSize.first, screenSize.second, "Testi-ikkuna");
	imagedata.drawImage(window);
	window.flush(Color(0));

	for (std::vector<int>& tile : tiles)
	{
		pool.push(tileTask, std::ref(imagedata), std::ref(scene), std::ref(tile));
	}

	auto prior = std::chrono::system_clock::now();
	auto now = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - prior);

	while (pool.nRun() > 0 && !window.windowShouldClose())
	{
		window.pollEvents();
		window.processInput();

		now = std::chrono::system_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - prior);
		if (elapsed.count() > 0) {
			imagedata.drawImage(window);
			window.flush(Color(0));
			prior = now;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	imagedata.drawImage(window);
	window.flush(Color(0));
	pool.stop(true); // wait for all the tasks in the pool to finish and stop

	auto stop = std::chrono::system_clock::now();
	auto renderElapsed = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
	std::cout << "Image rendered in " << renderElapsed.count() << " seconds!" << std::endl;
	std::string testout = "C:\\Users\\jaakk\\Pictures\\testout.bmp";
	imagedata.writeImage(testout);
	
	while (!window.windowShouldClose())
	{
		window.pollEvents();
		window.processInput();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	window.shutdown();
}




// draws scaled image to the middle of the window
void ImageData::drawImage(MinWindow& window) const
{
	int window_h = window.getHeight();
	int window_w = window.getWidth();
	int draw_w = window_w;
	int draw_h = window_h;
	double wRatio = window_w / (double)width;
	double hRatio = window_h / (double)height;
	if (wRatio < hRatio)
	{
		draw_h = int(height * wRatio);
	}
	else
	{
		draw_w = int(width * hRatio);
	}

	int index, dx, dy, wx, wy;
	Color c;
	for (int i = 0; i < draw_h; i++)
	{
		for (int j = 0; j < draw_w; j++)
		{
			dx = nearestNeighbor(i, height, draw_h);
			dy = nearestNeighbor(j, width, draw_w);

			index = (dx * width + dy) * 3;
			c.x = data[index + 2] / (double)255;
			c.y = data[index + 1] / (double)255;
			c.z = data[index] / (double)255;

			if (window_h > draw_h)
				wy = window_h - i - (window_h - draw_h) / 2;
			else
				wy = window_h - i;
			if (window_w > draw_w)
				wx = j + (window_w - draw_w) / 2;
			else
				wx = j;
			window.putPixel(wx, wy, c);
		}
	}
}