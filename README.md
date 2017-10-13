Nodable is a simple node based program !

First code example :

	cout << " -- Nodable v0.1 - by Berdal84 - 2017 --" << endl;

	// Create a context
	auto ctx    = new Node_Context("Global");

	// Create a simple graph that represent an addition named MyAddition
	{
		auto 		a 		= new Node_Integer(10);
		auto 		b 		= new Node_Integer(9);
		auto 		c 		= new Node_Integer();
		auto 		add 	= new Node_Add(a, b, c);
		Node_Tag* 	tag 	= new Node_Tag(ctx, "MyAddition", add);
	}

	// Try to find the node MyAddition and evaluate its value.
	auto node = ctx->find("MyAddition");
	if (node != nullptr){
    ((Node_Add*)node->getValue())->evaluate();
  }
