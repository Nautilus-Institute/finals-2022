package institute.nautilus.web4factory;

import org.springframework.expression.Expression;
import org.springframework.expression.ExpressionParser;
import org.springframework.expression.spel.standard.SpelExpressionParser;
import org.springframework.expression.spel.support.StandardEvaluationContext;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.lang.*;
import java.util.*;
import java.io.*;

public class Web4FactoryExpression implements java.io.Serializable {
    static Logger logger = LoggerFactory.getLogger(Web4FactoryExpression.class);
    private static final long serialVersionUID = 31337;

    public String expr;
    public Class type;

    public Web4FactoryExpression(String expr, Class type) {
        this.expr = expr;
        this.type = type;
    }

    public Object exec(Object input) {
        ExpressionParser parser = new SpelExpressionParser();

        StandardEvaluationContext context = new StandardEvaluationContext();
        context.setVariable("input", input);

        //logger.info("Starting to run expression `{}`", expr);

        Expression exp = parser.parseExpression(expr);
        Object res = exp.getValue(context);
        return res;
    }
}
