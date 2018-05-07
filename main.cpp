//
//  Display a rotating cube with lighting - Gouraud shading
//
//  Light and material properties are sent to the shader as uniform
//    variables.  Vertex positions and normals are sent as vertex attributes
//

#include "Angel.h"
#include "mat.h"
#include <iostream>
#include <string>
#include <fstream>
#include "vec.h"



#define v1 0
#define v2 1
#define v3 2
#define v4 3


using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
fstream file;
point4 *mvertices;
vec3  * mnormals;
vec3 *triangles;
point4 *pts;
vec3 *nors;
int numvertices, numtriangles;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
vec3   normals[NumVertices];
vec3* loadOff(std::string filename);
void populatePoints();

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices

int Index = 0;

void
quad( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edge to
    //   compute its face normal 
    vec4 u = vertices[b] - vertices[a];
    vec4 v = vertices[c] - vertices[b];

    vec3 normal = normalize( cross(u, v) );

    normals[Index] = normal; points[Index] = vertices[a]; Index++;
    normals[Index] = normal; points[Index] = vertices[b]; Index++;
    normals[Index] = normal; points[Index] = vertices[c]; Index++;
    normals[Index] = normal; points[Index] = vertices[a]; Index++;
    normals[Index] = normal; points[Index] = vertices[c]; Index++;
    normals[Index] = normal; points[Index] = vertices[d]; Index++;
    
    //Note that normals are fixed for a given face of the cube.
    //So the normal of a vertex is NOT computed based on neighboring faces.
    //which makes sense in this example since this is a cube with only 6 faces.
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
minit()
{
    colorcube();

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals),
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points),
		     sizeof(normals), normals );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( program, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

    // Initialize shader lighting parameters
    point4 light_position( -2.0, 0.0, 0.0, 1.0 );
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 ); // L_a
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 ); // L_d
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 ); // L_s

    color4 material_ambient( 1.0, 0.0, 1.0, 1.0 ); // k_a
    color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 ); // k_d
    color4 material_specular( 1.0, 0.8, 0.0, 1.0 ); // k_s
    float  material_shininess = 100.0;

    color4 ambient_product = light_ambient * material_ambient; // k_a * L_a
    color4 diffuse_product = light_diffuse * material_diffuse; // k_d * L_d
    color4 specular_product = light_specular * material_specular; // k_s * L_s
    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
		  1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
		  1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
		  1, specular_product );
	
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
		  1, light_position );

    glUniform1f( glGetUniformLocation(program, "Shininess"),
		 material_shininess );
		 
    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );

    glEnable( GL_DEPTH_TEST );

    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
}

//----------------------------------------------------------------------------










void
init()
{
	loadOff("shapeX.offx");
	populatePoints();
	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	
	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, numtriangles*3*16 + numtriangles*3*12,
		NULL, GL_STATIC_DRAW);
	cout << numtriangles*3*16 << numvertices << numtriangles << endl;
	

	glBufferSubData(GL_ARRAY_BUFFER, 0, numtriangles * 3 * 16, pts); 
	glBufferSubData(GL_ARRAY_BUFFER, numtriangles*3*16,
		numtriangles*3*12, nors);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(numtriangles*16*3));

	// Initialize shader lighting parameters
	point4 light_position(-2.0, 0.0, 0.0, 1.0);
	color4 light_ambient(0.2, 0.2, 0.2, 1.0); // L_a
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0); // L_d
	color4 light_specular(1.0, 1.0, 1.0, 1.0); // L_s

	color4 material_ambient(1.0, 0.0, 1.0, 1.0); // k_a
	color4 material_diffuse(1.0, 0.8, 0.0, 1.0); // k_d
	color4 material_specular(1.0, 0.8, 0.0, 1.0); // k_s
	float  material_shininess = 100.0;

	color4 ambient_product = light_ambient * material_ambient; // k_a * L_a
	color4 diffuse_product = light_diffuse * material_diffuse; // k_d * L_d
	color4 specular_product = light_specular * material_specular; // k_s * L_s
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"),
		1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"),
		1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"),
		1, specular_product);

	glUniform4fv(glGetUniformLocation(program, "LightPosition"),
		1, light_position);

	glUniform1f(glGetUniformLocation(program, "Shininess"),
		material_shininess);

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------










void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //  Generate tha model-view matrixn

    const vec3 viewer_pos( 0.0, 0.0, 3.0 );
    mat4  model_view = ( Translate( -viewer_pos ) *
			 RotateX( Theta[Xaxis] ) *
			 RotateY( Theta[Yaxis] ) *
			 RotateZ( Theta[Zaxis] ) );
    
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );

    glDrawArrays( GL_TRIANGLES, 0, numtriangles*3 );
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
	switch( button ) {
	    case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
	    case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
	    case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
	}
    }
}

//----------------------------------------------------------------------------

void
idle( void )
{
    Theta[Axis] += 0.01;

    if ( Theta[Axis] > 360.0 ) {
	Theta[Axis] -= 360.0;
    }
    
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
    mat4  projection = Perspective( 45.0, aspect, 0.5, 6.0 );

    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Gouraud" );

	glewInit();
	glewExperimental = GL_TRUE;

    init();

    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );
    glutMouseFunc( mouse );
    glutIdleFunc( idle );

    glutMainLoop();
    return 0;
}


void setupLoad(std::string filename) {

}

vec3* loadOff(std::string filename) {

	int i, num_vertices, num_triangles;
	string offx_word;
	
	


	file = fstream(filename);
	if (!file.is_open()) {
		return false;
	}

	file >> offx_word;
	cout << offx_word << endl;
	file >> num_vertices >> num_triangles >> i;


	
	const int mnumVertices = num_triangles * 3;
	numvertices = num_vertices;
	numtriangles = num_triangles;


	mvertices = new point4[numvertices];
	mnormals = new vec3[numvertices];
	triangles = new vec3[numtriangles];

	
	for (i = 0; i < num_vertices; i++) {
		GLfloat temp1, temp2, temp3;
		file >> mvertices[i][v1] >> mvertices[i][v2] >>mvertices[i][v3];
		//cout << mpoints[i][v1] << mpoints[i][v2] << mpoints[i][v3] << endl;
	}

	for (i = 0; i < num_triangles; i++)
	{
		GLfloat tempa, temp2, temp3, temp4;
		file >> tempa >> triangles[i][v1] >> triangles[i][v2] >> triangles[i][v3];
		//cout << tempa << endl;
	}

	for (i = 0; i < num_vertices; i++)
	{
		string tempa;
		GLfloat temp2, temp3, temp4;//vt
		file >> tempa >> temp2 >> temp3;
		//cout << tempa << endl;
	}

	

	for (i = 0; i < num_vertices; i++)
	{
		string tempa;//vn
		file >> tempa >> mnormals[i][v1] >> mnormals[i][v2] >> mnormals[i][v3];
		//cout << mnormals[i][v1] << mnormals[i][v2] << mnormals[i][v3] << endl;
		//cout << tempa << endl;
	}
	
	file.close();
	return mnormals;






}

void
populatePoints() {
	int i,index = 0;
	point4 temp1,temp2,temp3,temp4;
	vec3 nor1,nor2,nor3;
	pts = new point4[numtriangles * 3];
	nors = new vec3[numtriangles * 3];
	for (i = 0; i < numtriangles; i++) {
		temp1.x = mvertices[unsigned int(triangles[i][v1])].x;
		temp1.y = mvertices[unsigned int(triangles[i][v1])].y;
		temp1.z = mvertices[unsigned int(triangles[i][v1])].z;
		temp1.w = 1.0;

		temp2.x = mvertices[unsigned int(triangles[i][v2])].x;
		temp2.y = mvertices[unsigned int(triangles[i][v2])].y;
		temp2.z = mvertices[unsigned int(triangles[i][v2])].z;
		temp2.w = 1.0;

		temp3.x = mvertices[unsigned int(triangles[i][v3])].x;
		temp3.y = mvertices[unsigned int(triangles[i][v3])].y;
		temp3.z = mvertices[unsigned int(triangles[i][v3])].z;
		temp3.w = 1.0;

		pts[3 * i] = normalize(temp1);
		pts[(3 * i)+1] = normalize(temp2);
		pts[(3 * i)+2] = normalize(temp3);





		nor1.x = mnormals[unsigned int(triangles[i][v1])].x;
		nor1.y = mnormals[unsigned int(triangles[i][v1])].y;
		nor1.z = mnormals[unsigned int(triangles[i][v1])].z;
		

		nor2.x = mnormals[unsigned int(triangles[i][v2])].x;
		nor2.y = mnormals[unsigned int(triangles[i][v2])].y;
		nor2.z = mnormals[unsigned int(triangles[i][v2])].z;
		

		nor3.x = mnormals[unsigned int(triangles[i][v3])].x;
		nor3.y = mnormals[unsigned int(triangles[i][v3])].y;
		nor3.z = mnormals[unsigned int(triangles[i][v3])].z;
		

		nors[3 * i] = normalize(nor1);
		nors[(3 * i) + 1] = normalize(nor2);
		nors[(3 * i) + 2] = normalize(nor3);





		
	}



}
