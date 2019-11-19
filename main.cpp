#include "Angel.h"
#include <iostream>
#include <fstream>
using namespace std;

typedef vec4  color4;
typedef vec4  point4;

/*Functions and variables for initialization of the Sphere vertices*/

const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;
// (4 faces)^(NumTimesToSubdivide + 1) 
const int NumVertices_sphere = 3 * NumTriangles;

/*points_sphere will contains the vertices for rendering Sphere*/
point4 points_sphere[NumVertices_sphere];
vec3 normals[NumVertices_sphere];

//---------------------------------------------------------------------
int Index1 = 0;
void triangle(const point4& a, const point4& b, const point4& c) {
	vec3 normal = normalize(cross(b - a, c - b));
	normals[Index1] = normal; points_sphere[Index1] = a;
	Index1++; normals[Index1] = normal;
	points_sphere[Index1] = b; Index1++;
	normals[Index1] = normal;
	points_sphere[Index1] = c; Index1++;
}
//---------------------------------------------------------------------
point4 unit(const point4& p) {
	float len = p.x*p.x + p.y*p.y + p.z*p.z;
	point4 t; if (len > DivideByZeroTolerance) { t = p / sqrt(len); t.w = 1.0; }
	return t;
}
void divide_triangle(const point4& a, const point4& b, const point4& c, int count) {
	if (count > 0) {
		point4 v1 = unit(a + b);
		point4 v2 = unit(a + c);
		point4 v3 = unit(b + c);
		divide_triangle(a, v1, v2, count - 1);
		divide_triangle(c, v2, v3, count - 1);
		divide_triangle(b, v3, v1, count - 1);
		divide_triangle(v1, v3, v2, count - 1);
	}
	else {
		triangle(a, b, c);
	}
}

void tetrahedron(int count) {
	point4 v[4] = {
		vec4(0.0, 0.0, 1.0, 1.0),
		vec4(0.0, 0.942809, -0.333333, 1.0),
		vec4(-0.816497, -0.471405, -0.333333, 1.0),
		vec4(0.816497, -0.471405, -0.333333, 1.0)
	};
	divide_triangle(v[0], v[1], v[2], count);
	divide_triangle(v[3], v[2], v[1], count);
	divide_triangle(v[0], v[3], v[1], count);
	divide_triangle(v[0], v[2], v[3], count);
}
//---------------------------------------------------------------------

/*Functions and variables for initialization of Cube vertices */

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

/*points will contains the vertices for rendering Cube*/
point4 points[NumVertices];
/*colors will contains the color for each vertex for rendering Cube
Since points and colors are in the same size, each vertex will contain a color 
Each element in colors will correspond to each vertex in points array */
color4 colors[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5,  0.5, 1.0),
	point4(-0.5,  0.5,  0.5, 1.0),
	point4(0.5,  0.5,  0.5, 1.0),
	point4(0.5, -0.5,  0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5,  0.5, -0.5, 1.0),
	point4(0.5,  0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA olors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 1.0, 1.0),  // white
	color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;
GLuint vColor;
//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------
/*object_type == 0 corresponds to cube
  object_type == 1 corresponds to sphere*/
int object_type = 0;

/*vertex_colors2 contains colors for each vertex of Sphere*/
color4 vertex_colors2[NumVertices_sphere];


// Array of rotation angles (in degrees) for each coordinate axis
//Rotated bunny to have the correct angle to see its bouncing
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3};
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

int NumVertices_bunny;
/*bunny array contains the triangle vertices */
point4 bunny[29520];
/*colors_bun contains the color for each vertex*/
color4 colors_bun[29520];

/*Method where I load the bunny*/
void initialize_bunny() {
	
	char str[255];
	ifstream in("bunny.txt");
	if (!in) {
		cout << "Cannot open input file.\n";
	}

	in.getline(str, 255);
	in.getline(str, 255);
	char * numVertices_string = strtok(str, " ");
	char * numTriangles_string = strtok(NULL, " ");
	/*Read number of vertices and number of triangles*/
	const int number_bunny_vertices = atoi(numVertices_string);
	const int number_bunny_indexes = atoi(numTriangles_string);

	/*Bunny vertices*/
	point4 * bunny_vertices = (point4*)malloc(number_bunny_vertices * sizeof(point4));
	/*vertex_indexes contain each triangle's vertex as an index of bunny_vertices array */
	vec3 * vertex_indexes = (vec3*)malloc(number_bunny_indexes * sizeof(vec3));
	NumVertices_bunny = number_bunny_indexes * 3;
	for (int i = 3; i < number_bunny_vertices + 3; i++) {
		in.getline(str, 255);
		//if (in) cout << str << endl;
		char * x = strtok(str, " ");
		char * y = strtok(NULL, " ");
		char * z = strtok(NULL, " ");
		float x_coord = atof(x);
		float y_coord = atof(y);
		float z_coord = atof(z);
		point4 point = point4(x_coord, y_coord, z_coord, 1.0);
		bunny_vertices[i - 3] = point;
	}
	for (int i = number_bunny_vertices + 3; i < number_bunny_vertices + 3 + number_bunny_indexes; i++) {
		in.getline(str, 255);  
		//if (in) cout << str << endl;
		char * empty3 = strtok(str, " ");
		char * x = strtok(NULL, " ");
		char * y = strtok(NULL, " ");
		char * z = strtok(NULL, " ");
		int x_ind = atoi(x);
		int y_ind = atoi(y);
		int z_ind = atoi(z);
		//printf("\nX_ind: %d\n", x_ind);
		vec3 vertex = vec3(x_ind, y_ind, z_ind);
		vertex_indexes[i - (number_bunny_vertices + 3)] = vertex;
	}
	/*Here bunny triangle vertices array is filled using bunny_vertices and vertex_indexes */
	int counter = 0;
	for (int i = 0; i < number_bunny_indexes; i++) {
		int first_ind = vertex_indexes[i].x;
		int second_ind = vertex_indexes[i].y;
		int third_ind = vertex_indexes[i].z;
		point4 point1 = bunny_vertices[first_ind];
		point4 point2 = bunny_vertices[second_ind];
		point4 point3 = bunny_vertices[third_ind];
		bunny[counter++] = point1;
		bunny[counter++] = point2;
		bunny[counter++] = point3;
	}
	in.close();
	/*Here I gave correct angle to rotation to rotate the bunny in a correct angel */
	Theta[Yaxis] = 0;
	Theta[Zaxis] = 90;
	Theta[Xaxis] = -90;
}

// OpenGL initialization
void
init()
{
	initialize_bunny();
	//create cube vertices array
	colorcube();
	//create sphere vertices array
	tetrahedron(NumTimesToSubdivide);
	/*Initialize arrays containing color values for sphere and cube
	  Initialized as red for both sphere and cube */
	for (int i = 0; i < NumVertices_sphere; i++) {
		vertex_colors2[i] = color4(1.0, 0.0, 0.0, 1.0);
	}
	for (int i = 0; i < NumVertices; i++) {
		colors[i] = color4(1.0, 0.0, 0.0, 1.0);
	}
	for (int i = 0; i < NumVertices_bunny; i++) {
		colors_bun[i] = color4(1.0, 0.0, 0.0, 1.0);
	}
	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	/*When the program first initialized, it will start with the cube
	  because the data of cube, vertecies array, points, and colors array is passed to the buffer */
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
	
	// Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");

	// Set projection matrix
	mat4  projection;
	projection = Ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); // Ortho(): user-defined function in mat.h
	//projection = Perspective( 45.0, 1.0, 0.5, 3.0 );
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}
//----------------------------------------------------------------------------

/*Global variables related to bouncing shape */

//velocity is the velocity on the y axis
float velocity = 0.0008;
/*when ball bounces up velocity will change its sign, 
  however abs_velocity, absolute velocity, will always be positive */
float abs_velocity = 0.0008;
/*acceleration is the gravity */
/*abs_velocity will be decreased with each movement because of the acceleration on the 
  opposite direction */
float acceleration = 0.0000004;
/*Since velocity will change with acceleration, I needed to keep initial velocity for reshape events
  Or when I initialize scene with different shape or different horizontal velocity from left top */
float initial_velocity = 0.0008;
//x and y contains the displacement to Translate in the display function
float x = 0;
float y = 0;
//flag1 is for understanding whether the shape is bouncing up or down
int flag1 = 0;
//energy is going to decrease with each bounce on the ground and when energy is zero, the ball stop
float energy = 1;
//horizontal_velocity can be changed with user input
float horizontal_velocity = 0.00009;
//height is the height where the shape started bouncing
float height = 1.6;

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//  Generate tha model-view matrix
	const vec3 displacement(0.0, 0.0, 0.0);
	mat4  model_view;
	//For translation of the shape, x and y changes in the idle method
	if (object_type == 0 || object_type == 1)
		model_view = Translate(x, y, 0)*Translate(-0.7, 0.7, 0)* Scale(0.2, 0.2, 0.2);
	else
		model_view = Translate(x, y, 0)*Translate(-0.7, 0.7, 0)* Scale(0.01, 0.01, 0.01)*
		   (RotateX(Theta[Xaxis]) *
			RotateY(Theta[Yaxis]) *
			RotateZ(Theta[Zaxis]));

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	/*According to the object type, the number of vertices to draw differs
	  Sphere contains a lot more vertices than cube */
	if(object_type==0)
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	else if(object_type==1)
		glDrawArrays(GL_TRIANGLES, 0, NumVertices_sphere);
	else if (object_type == 2)
		glDrawArrays(GL_TRIANGLES, 0, NumVertices_bunny);
	glutSwapBuffers();

}
//----------------------------------------------------------------------------
void
mouse(int button, int state, int x, int y)
{
}
//----------------------------------------------------------------------------

void
idle(void)
{
	//when energy is small enough, do not change x and y values
	//x and y values are given to Translation in display method
	//So the shape does not move and stops
	if (energy <= 0.00002) {
	}
	else {
		/*x value of translation is increased with constant horizontal velocity */
		x += horizontal_velocity;
		/*When shape comes to the ground*/
		//When shape's y is the same as the height of the window, which means shape is on the ground
		if (y <= -height) {
			//absolute velocity decreases because some energy is lost in contact with the ground
			abs_velocity *= 0.85;
			//velocity changes direction, now upwards previously velocity was downwards
			velocity = -abs_velocity;
			flag1 = 1;
			//energy decreases with each touch on the ground
			energy = energy / 2.5;
		}
		/*When shape does not have any velocity to go up it starts acceleration downwards*/
		else if (velocity >= 0 && flag1 == 1) {
			velocity = abs_velocity;
			flag1 = 0;
		}
		//So here velocity and acceleration in the same sign, + and + when the shape goes down
		//Which means velocity will increase while going down with acceleration
		//When the shape going upwards velocity and acceleration have different signs - and +
		//Because I changed the sign of the velocity in the above if statement when touching ground
		//Velocity will be decreasing while the shape is going up
		velocity += acceleration;
		//In each idle method is called, y position of the shape will be changed by velocity
		y -= velocity;
	}
	glutPostRedisplay();
}

//----------------------------------------------------------------------------
void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}
//----------------------------------------------------------------------------

/*drawCube and drawSphere methods change the buffer's data bind to VAO
  So when I change the data bind to the buffer and VAO, in the next display method call
  The rendering will be based on this new data
  So cube and sphere will be rendered
*/
void drawCube() {
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));
	height = 1.6;
 }
void drawSphere() {
	glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(vertex_colors2), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_sphere), points_sphere);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere), sizeof(vertex_colors2), vertex_colors2);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points_sphere)));
	height = 1.5;
}
void drawBunny() {
	glBufferData(GL_ARRAY_BUFFER, sizeof(bunny) + sizeof(colors_bun),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bunny), bunny);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(bunny), sizeof(colors_bun), colors_bun);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(colors_bun)));
	height = 1.7;
}
/*This method reseting every value to initial so that bouncing starts from the left top again */
void initialize(float horizontal) {
	x = 0;
	y = 0;
	velocity = initial_velocity;
	abs_velocity = initial_velocity;
	energy = 1;
	flag1 = 0;
	horizontal_velocity = horizontal;
}
/*This method changes the color arrays of the cube and the sphere to the specified color*/
void changeColorTo(color4 color_vector) {
	for (int i = 0; i < NumVertices_sphere; i++) {
		vertex_colors2[i] = color_vector;
	}
	for (int i = 0; i < NumVertices; i++) {
		colors[i] = color_vector;
	}
	for (int i = 0; i < NumVertices_bunny; i++) {
		colors_bun[i] = color_vector;
	}
}
/*When pressed i on the keyboard: The scene is initialized from the left top position
  When pressed h: Explanation of the inputs are printed on the command line
  When pressed q: Exit the program
*/
void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033:  // Escape key
	case 'i': case 'I':
		initialize(0.0001);
		break;
	case 'h': case 'H':
		printf("With left click, a velocity menu will pop up.\n");
		printf("With this menu, you can specify the initial horizontal velocity to 9, 10 or 20 or 30 or 40. \n");
		printf("With righ click, a general menu, which contains shape, polygon and color sub menus, will pop up.\n");
		printf("With polygon menu, you can draw your shape as wireframe or solid.\n");
		printf("With shape menu, you can choose the shape as one of cube, sphere or a bunny.\n");
		printf("With color menu, you can choose the color of your shape.\n");
		printf("Velocity menu and shape menu will start the bouncing from the beginning which is the left top.\n");
		printf("When you change the polygon mode and color, the scene will not start from the beginning, it will only continue with the specified polygon mode and color.\n");
		printf("So, when you change the color of a shape, it will not start from the leftmost, it will continue its movement from where it was with new color.\n");
		printf("When 9 and 10 chosen as the velocities, you can observe the stopping of the shape.\n");
		printf("With 20, 30 and 40 velocities, shape will continue its bouncing out of the window.\n");
		break;
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	}
}
//----------------------------------------------------------------------------
/*Here menu functions start */


void colorMenu(int id)
/* This is the callback function for the color menu. */
{
	switch (id)
	{
	case 1: /* change color to green */
		changeColorTo(color4(0.0, 1.0, 0.0, 1));
		/*Each time color array changes, the data of the new colors should be sent to the VAO.
		  Changing the binding data with the glBufferData and glBufferSubData is done in the
		  drawCube and drawSphere methods*/
		//If object type is cube, send the new color data to the binded buffer with drawCube method, I created above
		if (object_type == 0)
			drawCube();
		else if (object_type == 1)
			drawSphere();
		else
			drawBunny();
		break;
	case 2: /* change color to red */
		changeColorTo(color4(1.0, 0.0, 0.0, 1));
		if (object_type == 0)
			drawCube();
		else if (object_type == 1)
			drawSphere();
		else
			drawBunny();
		break;
	case 3: /* change color to blue */
		changeColorTo(color4(0.0, 0.0, 1.0, 1));
		if (object_type == 0)
			drawCube();
		else if (object_type == 1)
			drawSphere();
		else
			drawBunny();
		break;
	case 4: /* change color to yellow */
		changeColorTo(color4(1.0, 1.0, 0.0, 1));
		if (object_type == 0)
			drawCube();
		else if (object_type == 1)
			drawSphere();
		else
			drawBunny();
		break;
	case 5: /* change color to magenta */
		changeColorTo(color4(1.0, 0.0, 1.0, 1.0));
		if (object_type == 0)
			drawCube();
		else if (object_type == 1)
			drawSphere();
		else
			drawBunny();
		break;
	case 6: /* change color to cyan */
		changeColorTo(color4(0.0, 1.0, 1.0, 1.0));
		if (object_type == 0)
			drawCube();
		else if (object_type == 1)
			drawSphere();
		else
			drawBunny();
		break;
	case 7: /* change color to white */
		changeColorTo(color4(1.0, 1.0, 1.0, 1.0));
		if (object_type == 0)
			drawCube();
		else if (object_type == 1)
			drawSphere();
		else
			drawBunny();
		break;
	case 8: /* change color to black */
		changeColorTo(color4(0.0, 0.0, 0.0, 1.0));
		if (object_type == 0)
			drawCube();
		else if (object_type == 1)
			drawSphere();
		else
			drawBunny();
		break;
	default: /* for any case not covered above, leave color unchanged */
		break;
	}
}

/*Horizontal velocity of the movement decided on a menu*/
void velocityMenu(int id)
/* This is the callback function for the horizontal velocity menu. */
{
	switch (id)
	{
	case 1: /* change velocity to 0.00009 */
		initialize(0.00009);
		break;
	case 2: /* change velocity to 0.00010 */
		initialize(0.0001);
		break;
	case 3: /* change velocity to 0.00020 */
		initialize(0.0002);
		break;
	case 4: /* change velocity to 0.00030 */
		initialize(0.0003);
		break;
	case 5: /* change velocity to 0.00040 */
		initialize(0.0004);
		break;
	default:
		break;
	}
}

/*In this method object_type global variable is changed */
void shapeMenu(int id)
/* This is the callback function for the shape, sphere or cube, menu. */
{
	switch (id)
	{
	case 1: /* change to cube */
		object_type = 0;
		initialize(0.00009);
		drawCube();
		break;
	case 2: /* change to sphere */
		object_type = 1;
		initialize(0.00009);
		drawSphere();
		break;
	case 3: /* change to sphere */
		object_type = 2;
		initialize(0.00009);
		drawBunny();
		break;
	default:
		break;
	}
}

/*Polygon mode menu */
void fillingMenu(int id)
/* This is the callback function for the shape, solid or wireframe, menu. */
{
	switch (id)
	{
	case 1: /* change to solid */
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 2: /* change to wireframe */
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	default:
		break;
	}
}
void Menu(int id) 
{
}

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Bouncing Shape");

	printf("Supported GLSL version is %s.\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("%s\n%s\n%s\n",
		glGetString(GL_RENDERER),  // e.g. Intel HD Graphics 3000 OpenGL Engine
		glGetString(GL_VERSION),    // e.g. 3.2 INTEL-8.0.61
		glGetString(GL_SHADING_LANGUAGE_VERSION));

	glewExperimental = GL_TRUE;
	glewInit();
	
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);

	/* Added menus */
	int velocity_menu = glutCreateMenu(velocityMenu);
	glutAddMenuEntry("9",  1);
	glutAddMenuEntry("10", 2);
	glutAddMenuEntry("20", 3);
	glutAddMenuEntry("30", 4);
	glutAddMenuEntry("40", 5);
	glutAttachMenu(GLUT_LEFT_BUTTON);

	int shape_menu = glutCreateMenu(shapeMenu);
	glutAddMenuEntry("Cube", 1);
	glutAddMenuEntry("Sphere", 2);
	glutAddMenuEntry("Bunny", 3);

	int filling_menu = glutCreateMenu(fillingMenu);
	glutAddMenuEntry("Solid", 1);
	glutAddMenuEntry("Wireframe", 2);


	int color_menu = glutCreateMenu(colorMenu);
	glutAddMenuEntry("Green", 1);
	glutAddMenuEntry("Red", 2);
	glutAddMenuEntry("Blue", 3);
	glutAddMenuEntry("Yellow", 4);
	glutAddMenuEntry("Magenta", 5);
	glutAddMenuEntry("Cyan", 6);
	glutAddMenuEntry("White", 7);
	glutAddMenuEntry("Black", 8);

	int menu = glutCreateMenu(Menu);
	glutAddSubMenu("Polygon Mode", filling_menu);
	glutAddSubMenu("Shape Menu", shape_menu);
	glutAddSubMenu("Color Menu", color_menu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();

	return 0;
}
