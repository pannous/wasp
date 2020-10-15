//
// Created by pannous on 12.06.20.
//



void testFunctionParams(){
	assert_equals(parse("f(x)=x*x").param->first(),"x");
	assert_equals("f(x)=x*x;f(3)","9");// functions => angle!
}
void testAllAngle(){
	testFunctionParams();
}

void testAngle() {
//	testAllAngle();
}